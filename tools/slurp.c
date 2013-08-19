#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MEMORY_CHUNK 1000000 // 1M

// slurp file
int slurp(const char *filename, unsigned char **buffer, size_t maxsize)
{
  FILE *f = NULL;
  size_t bufsize = 0;
  *buffer = NULL;
  int ret = -255;

  if((f = fopen(filename, "r")) == NULL)
  {
    ret = -1;
    goto error;
  }

  while(1)
  {
    unsigned char *ptr;
    size_t oldsize = bufsize;
    size_t newsize = bufsize+MEMORY_CHUNK;
    size_t ret;

    if(maxsize > 0 && newsize > maxsize)
      newsize = maxsize;

    if((ptr = (unsigned char *)realloc(*buffer, newsize)) == NULL)
    {
      ret = -2;
      goto error;
    }
    else
    {
      *buffer = ptr;
      bufsize = newsize;
    }

    if((ret = fread(&ptr[oldsize], 1, newsize-oldsize, f)) < (newsize-oldsize))
    {
      fclose(f);
      return oldsize+ret;
    }

    if(maxsize > 0 && bufsize == maxsize)
    {
      ret = -3;
      goto error;
    }
  }

  error:
    if(f != NULL)
      fclose(f);
    if(*buffer != NULL)
    {
      free(*buffer);
      *buffer = NULL;
    }

    return ret;
}
