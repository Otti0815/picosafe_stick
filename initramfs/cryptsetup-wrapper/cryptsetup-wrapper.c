#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CRYPTSETUP "/sbin/cryptsetup"

int main(int argc, char *argv[])
{
  char *device = argv[1];
  char *name   = argv[2];

  char * const argv_cryptsetup[] = {
    CRYPTSETUP,
    "luksOpen",
    device,
    name,
    NULL
  };

  if(argc < 3)
  {
    fprintf(stderr, "cryptsetup-wrapper: %s <device> <name>\n", argv[0]);
    return 1;
  }

  setuid(0);
  setgid(0);

  return execvp(CRYPTSETUP, argv_cryptsetup);
}
