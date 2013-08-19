#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RANDOMDEVICE "/dev/random"
#define PICOSAFE_SUFFIX "_picosafe.key"
#define LPC_SUFFIX      "_lpc.key"

void usage(int argc, char *argv[])
{
  printf("%s: [OPTIONS] FILE\n", argv[0]);
  printf("  This program will create AES key files for lpc-tools and picosafe tools.\n");
  printf("  The files created are FILE%s and FILE%s.\n\n", PICOSAFE_SUFFIX, LPC_SUFFIX);

  printf("OPTIONS\n");
  printf("  -h:          print this help\n");
}

int main(int argc, char *argv[])
{
  FILE *fh = NULL;
  unsigned char key[16];
  int i;
  char *output = NULL;
  char *file_lpc = NULL, *file_picosafe = NULL;

  /* parsing command line arguments */
  {
    char c;
    int hflag = 0;

    opterr = 0;
    while((c = getopt(argc, argv, "o:h")) != -1)
    {
      switch (c) 
      {   
        case 'h': hflag = 1;       break;
      }
    }
    
    /* print usage */
    if(hflag)
    {
      usage(argc, argv);
      return 0;
    }

    /* filename */
    if(optind >= argc)
    {
      fprintf(stderr, "suffix for filenames missing.\n");
      goto error;
    }
    else
      output = argv[optind];
  }

  if((fh = fopen(RANDOMDEVICE, "r")) == NULL)
  {
    fprintf(stderr, "Can't open %s for reading: %s\n", RANDOMDEVICE, strerror(errno));
    goto error;
  }

  if(fread(key, sizeof(key), 1, fh) != 1)
  {
    fprintf(stderr, "Can't read from %s: %s\n", RANDOMDEVICE, strerror(errno));
    goto error;
  }

  fclose(fh);

  //%s_picosafe.key 
  //%s_lpc.key 
  file_lpc      = malloc( strlen(output) + strlen(LPC_SUFFIX)      +1 );
  file_picosafe = malloc( strlen(output) + strlen(PICOSAFE_SUFFIX) +1 );

  if(file_lpc == NULL || file_picosafe == NULL)
  {
    fprintf(stderr, "Can't allocate memory.\n");
    goto error;
  }

  strcpy(file_lpc,      output);
  strcpy(file_picosafe, output);

  strcat(file_lpc,      LPC_SUFFIX);
  strcat(file_picosafe, PICOSAFE_SUFFIX);

  if((fh = fopen(file_lpc, "w")) == NULL)
  {
    fprintf(stderr, "Can't open %s for writing: %s\n", file_lpc, strerror(errno));
    goto error;
  }

  if(fwrite(key, sizeof(key), 1, fh) != 1)
  {
    fprintf(stderr, "Can't write key to %s: %s\n", file_lpc, strerror(errno));
    goto error;
  }

  fclose(fh);

  if((fh = fopen(file_picosafe, "w")) == NULL)
  {
    fprintf(stderr, "Can't open %s for writing: %s\n", file_picosafe, strerror(errno));
    goto error;
  }

  fprintf(fh, "0x");
  for(i = 0; i < sizeof(key); i++)
    fprintf(fh, "%02X", key[i]);

  fclose(fh);

  return 0;

  error:
    if(fh != NULL)
      fclose(fh);
    if(file_lpc != NULL)
      free(file_lpc);
    if(file_picosafe != NULL)
      free(file_picosafe);

    return 1;
}
