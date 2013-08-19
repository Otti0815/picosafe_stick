/*
 *  RSA 1024bit key generation program
 *  This program is based on an example program of the PolarSSL library.
 *
 *  Copyright (C) 2006-2011, Brainspark B.V.
 *  Copyright (C) 2012, Michael Hartmann <hartmann@embedded-projects.net>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "polarssl/config.h"

#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/bignum.h"
#include "polarssl/x509.h"
#include "polarssl/rsa.h"

/* keysize and exponent */
#define KEY_SIZE 1024
#define EXPONENT 65537

/* filenames */
#define PICOSAFE_PUB1 "picosafe_pub.txt"
#define PICOSAFE_PUB2 "pubkey.h"
#define PICOSAFE_PRIV "picosafe_priv.txt"

/* usage: information on how to use this program */
void usage(int argc, char *argv[])
{
  printf("Usage: %s [outputdir]\n", argv[0]);
  printf("  This program will create a RSA 1024bit key pair. The public and\n");
  printf("  private keys are written to the files %s, %s\n", PICOSAFE_PUB1, PICOSAFE_PUB2);
  printf("  (public key) and %s (private key).\n\n", PICOSAFE_PRIV);

  printf("  If no output directory is given, the current directory will be used.\n\n");

  printf("  The private key (%s) can be used to sign files using\n", PICOSAFE_PRIV);
  printf("  picosafe_sign.\n");
  printf("  The public key can be used to verify signatures. The file pubkey.h can\n");
  printf("  be used by the apex bootloader to verify the signature of a kernel.\n");
  printf("  Please read the picosafe manual on apex for further information.\n");
}

int main(int argc, char *argv[])
{
  int ret;
  rsa_context rsa;
  entropy_context entropy;
  ctr_drbg_context ctr_drbg;
  char *fpub1_filename = NULL;
  char *fpub2_filename = NULL;
  char *fpriv_filename = NULL;
  FILE *fpub1 = NULL;
  FILE *fpub2 = NULL;
  FILE *fpriv = NULL;
  char *outputdir = NULL;

  /* we have a command line argument */
  if(argc > 1)
  {
    /* check if user wants to see usage */
    if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
      usage(argc, argv);
      return 0;
    }

    /* apparently the command line argument is a directory */
    outputdir = argv[1];

    /* allocate memory for filenames */
    fpub1_filename = (char *)malloc(strlen(outputdir)+strlen(PICOSAFE_PUB1)+2);
    fpub2_filename = (char *)malloc(strlen(outputdir)+strlen(PICOSAFE_PUB2)+2);
    fpriv_filename = (char *)malloc(strlen(outputdir)+strlen(PICOSAFE_PRIV)+2);

    if(fpub1_filename == NULL || fpub2_filename == NULL || fpriv_filename == NULL)
    {
      printf("Can't allocate memory: %s\n", strerror(errno));
      ret = 1;
      goto exit;
    }

    /* fpub1_filename = outputdir + "/" + PICOSAFE_PUB1 */
    strcpy(fpub1_filename, outputdir);
    strcat(fpub1_filename, "/");
    strcat(fpub1_filename, PICOSAFE_PUB1);

    /* fpub2_filename = outputdir + "/" + PICOSAFE_PUB2 */
    strcpy(fpub2_filename, outputdir);
    strcat(fpub2_filename, "/");
    strcat(fpub2_filename, PICOSAFE_PUB2);

    /* fpriv_filename = outputdir + "/" + PICOSAFE_PRIV */
    strcpy(fpriv_filename, outputdir);
    strcat(fpriv_filename, "/");
    strcat(fpriv_filename, PICOSAFE_PRIV);
  }
  else
  {
    fpub1_filename = PICOSAFE_PUB1;
    fpub2_filename = PICOSAFE_PUB2;
    fpriv_filename = PICOSAFE_PRIV;
  }

  /* try to open the files */
  if((fpub1 = fopen(fpub1_filename, "wb+")) == NULL)
  {
    printf("Can't open %s for writing: %s\n", fpub1_filename, strerror(errno));
    ret = 1;
    goto exit;
  }

  if((fpub2 = fopen(fpub2_filename, "wb+")) == NULL)
  {
    printf("Can't open %s for writing: %s\n", fpub2_filename, strerror(errno));
    ret = 1;
    goto exit;
  }

  if((fpriv = fopen(fpriv_filename, "wb+")) == NULL)
  {
    printf("Can't open %s for writing: %s\n", fpriv_filename, strerror(errno));
    ret = 1;
    goto exit;
  }

  entropy_init(&entropy);
  if((ret = ctr_drbg_init(&ctr_drbg, entropy_func, &entropy, NULL, 0)) != 0)
  {
    printf("ctr_drbg_init failed: %d\n", ret);
    goto exit;
  }

  /* init rsa */
  rsa_init(&rsa, RSA_PKCS_V15, 0);
  
  /* generate the key pair */
  if((ret = rsa_gen_key(&rsa, ctr_drbg_random, &ctr_drbg, KEY_SIZE, EXPONENT)) != 0)
  {
    printf("rsa_gen_key failed: %d\n", ret);
    goto exit;
  }

  /* write to files */
  if((ret = mpi_write_file("N = ", &rsa.N, 16, fpub1)) != 0 ||
     (ret = mpi_write_file("E = ", &rsa.E, 16, fpub1)) != 0)
  {
    printf("Couldn't write to %s: mpi_write_file returned %d\n", fpub1_filename, ret);
    goto exit;
  }

  /* generate the header file with public key */
  mpi_write_file("const char PUBLIC_KEY_N[] = \"", &rsa.N, 16, fpub2);
  fseek(fpub2, -2, SEEK_CUR);
  fprintf(fpub2, "\";\n");
  mpi_write_file("const char PUBLIC_KEY_E[] = \"", &rsa.E, 16, fpub2);
  fseek(fpub2, -2, SEEK_CUR);
  fprintf(fpub2, "\";\n");

  /* write private key */
  if((ret = mpi_write_file("N = " , &rsa.N , 16, fpriv)) != 0 ||
     (ret = mpi_write_file("E = " , &rsa.E , 16, fpriv)) != 0 ||
     (ret = mpi_write_file("D = " , &rsa.D , 16, fpriv)) != 0 ||
     (ret = mpi_write_file("P = " , &rsa.P , 16, fpriv)) != 0 ||
     (ret = mpi_write_file("Q = " , &rsa.Q , 16, fpriv)) != 0 ||
     (ret = mpi_write_file("DP = ", &rsa.DP, 16, fpriv)) != 0 ||
     (ret = mpi_write_file("DQ = ", &rsa.DQ, 16, fpriv)) != 0 ||
     (ret = mpi_write_file("QP = ", &rsa.QP, 16, fpriv)) != 0)
  {
    printf("Can't write to %s: %d\n", fpriv_filename, ret);
    goto exit;
  }


/* exit the program and clean up */
exit:
  if(fpub1 != NULL)
    fclose(fpub1);

  if(fpub2 != NULL)
    fclose(fpub2);

  if(fpriv != NULL)
    fclose(fpriv);

  if(outputdir != NULL)
  {
    if(fpub1_filename != NULL)
      free(fpub1_filename);

    if(fpub2_filename != NULL)
      free(fpub2_filename);

    if(fpriv_filename != NULL)
      free(fpriv_filename);
  }

  rsa_free(&rsa);

  return ret;
}
