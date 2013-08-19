#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "polarssl/config.h"
#include "polarssl/aes.h"

#include "slurp.h"

/* keysize is 128bit / 16bytes */
#define KEYSIZE 128

#define DEFAULT_KEY "key.txt"

const unsigned char magic[] = { 0xe6, 0x8, 0xdf, 0xce, 0x79, 0xd0, 0x52, 0x8c, 0xbd, 0x67, 0x53, 0xc1, 0x77, 0x79, 0x5c, 0xd4 };


/* print usage */
void usage(int argc, char *argv[])
{
  printf("%s: [OPTIONS] FILE\n", argv[0]);
  printf("  This program will encrypt/decrypt FILE.\n\n");

  printf("OPTIONS\n");
  printf("  -k FILENAME: use key stored in FILENAME (if omitted, will try %s)\n", DEFAULT_KEY);
  printf("  -o FILENAME: write output to FILENAME (if omitted, FILE will be overwritten)\n");
  printf("  -d:          don't encrypt, but decrypt FILE\n");
  printf("  -h:          print this help\n");
}


static void reverse(unsigned char *string, int len)
{
  int i;
  unsigned char temp;

  for(i = 0; i < len/2; i++)
  {
    temp = string[len-i-1];
    string[len-i-1] = string[i];
    string[i] = temp;
  }
}


/* convert hex character to number */
int todec(int c)
{
  switch(tolower(c))
  {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;
    default:  return -1;
  }
}

int read_key(const char *filename, unsigned char key[16])
{
  unsigned char *content;
  int len, i;

  len = slurp(filename, &content, 64);

  if(len >= 34 && content[0] == '0' && content[1] == 'x')
  {
    for(i = 2; i < 34; i++)
      if(isxdigit(content[i]) == 0)
      {
        free(content);
        return 0;
      }

    /* store into key */
    for(i = 0; i < 16; i++)
      key[i] = todec(content[2+2*i])*16+todec(content[2+2*i+1]);

    reverse(key, 16);
    free(content);
    return 1;
  }
  else if(len >= 16)
  {
    memcpy(key, content, 16);
    reverse(key, 16);
    free(content);
    return 2;
  }
  else
  {
    if(content != NULL)
      free(content);
    return 0;
  }
}

int main(int argc, char *argv[])
{
  aes_context ctx;
  unsigned char *buffer = NULL;
  unsigned char key[KEYSIZE/8];
  unsigned char iv1[16], iv2[16];
  int lenin, mode, ret, i, c;
  FILE *fout    = NULL; /* filehandle writing to output file */
  char *keyfile = NULL; /* filename of keyfile*/
  char *fileout = NULL; /* output filename */
  char *filein  = NULL; /* input filename */
  int hflag = 0;        /* help flag */
  int dflag = 0;        /* decrypt flag */

  /* init srand */
  srand(time(NULL));

  /* parsing command line arguments */
  {
    opterr = 0;
    while((c = getopt(argc, argv, "dk:o:h")) != -1)
    {
      switch (c) 
      {   
        case 'h': hflag = 1;        break;
        case 'd': dflag = 1;        break;
        case 'k': keyfile = optarg; break;
        case 'o': fileout = optarg; break;
      }
    }
    
    /* print usage */
    if(hflag)
    {
      usage(argc, argv);
      return 0;
    }

    /* default keyfile */
    if(keyfile == NULL)
      keyfile = DEFAULT_KEY;

    /* mode - decrypt or encrypt */
    if(dflag)
      mode = AES_DECRYPT;
    else
      mode = AES_ENCRYPT;

    /* filename */
    if(optind >= argc)
    {
      fprintf(stderr, "Filename missing.\n");
      goto error;
    }
    else
      filein = argv[optind];

    /* overwrite file */
    if(fileout == NULL)
      fileout = filein;
  }

  /* read key from keyfile and store into key */
  if(read_key(keyfile, key) <= 0)
  {
    fprintf(stderr, "Invalid file format for keyfile\n");
    goto error;
  }

  /* create random iv */
  for(i = 0; i < sizeof(iv1); i++)
    iv1[i] = iv2[i] = rand();

  /* read input file */
  if((lenin = slurp(filein, &buffer, 0)) == 0)
    goto error;

  /* set key */
  if(mode == AES_ENCRYPT)
  {
    /* for encrypting */
    if((ret = aes_setkey_enc(&ctx, key, KEYSIZE)) != 0)
    {
      fprintf(stderr, "Can't set key: aes_setkey_enc %d\n", ret);
      goto error;
    }
  }
  else
  {
    /* for decrypting */
    if((ret = aes_setkey_dec(&ctx, key, KEYSIZE)) != 0)
    {
      fprintf(stderr, "Can't set key: aes_setkey_dec %d\n", ret);
      goto error;
    }
  }

  /* add magic and padding */
  if(mode == AES_ENCRYPT)
  {
    unsigned char *ptr;
    int newlen, padding = 1;
    
    newlen = lenin + sizeof(magic); // add magic bytes
    newlen += 1;                    // add one byte that contains the number of padding bytes
    if(newlen % 16 != 0)            // add padding bytes
    {
      int newlen_with_padding = ((newlen/16)+1)*16;
      padding = 1+newlen_with_padding - newlen;
      newlen = newlen_with_padding;
    }

    if((ptr = realloc(buffer, newlen)) == NULL)
    {
      fprintf(stderr, "Can't allocate memory: %s\n", strerror(errno));
      goto error;
    }
    else
      buffer = ptr;

    memcpy(&buffer[lenin], magic, sizeof(magic));     // append magic
    memset(&buffer[lenin+sizeof(magic)], 0, padding); // append padding bytes
    buffer[newlen-1] = padding;                       // number of padded bytes
    lenin = newlen;

    /* reverse every input block */
    {
      int j;
      for(j = 0; j <= lenin-16; j+=16)
        reverse(&buffer[j], 16);
    }
  }
  else if(mode == AES_DECRYPT)
  {
    /* copy iv */
    memcpy(iv1, &buffer[lenin-sizeof(iv1)], sizeof(iv1));
    lenin -= sizeof(iv1);
  }

  /* do en-/decryption */
  if((ret = aes_crypt_cbc(&ctx, mode, lenin, iv1, buffer, buffer)) != 0)
  {
    fprintf(stderr, "Can't %s: aes_crypt_cbc %d\n", mode == AES_ENCRYPT ? "encrypt" : "decrypt", ret);
    goto error;
  }

  if(mode == AES_DECRYPT)
  {
    unsigned int padding = buffer[lenin-1];

    if(padding > 16 || lenin < padding)
    {
      fprintf(stderr, "Invalid padding\n");
      goto error;
    }

    lenin -= padding;

    if(memcmp(&buffer[lenin-sizeof(magic)], magic, sizeof(magic)) != 0)
    {
      fprintf(stderr, "Input file invalid or invalid key\n");
      goto error;
    }

    lenin -= sizeof(magic);
  }

  /* write out result */
  if((fout = fopen(fileout, "w")) == NULL)
  {
    fprintf(stderr, "Can't open %s for writing: %s\n", fileout, strerror(errno));
    goto error;
  }

  /* reverse every encrypted block */
  {
    int j;
    for(j = 0; j < lenin; j+=16)
      reverse(&buffer[j], 16);
  }


  // append iv
  if(mode == AES_ENCRYPT)
  {
    reverse(iv2, sizeof(iv2));
    fwrite(iv2, 1, sizeof(iv2), fout);
  }

  fwrite(buffer, 1, lenin, fout);

  fclose(fout);
  free(buffer);

  return 0;

  error:
    if(fout != NULL)
      fclose(fout);

    if(buffer != NULL)
      free(buffer);

    return 1;
}
