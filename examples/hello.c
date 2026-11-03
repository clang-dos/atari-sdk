// Prints a greeting with the C library, its arguments, and the TOS and
// GEMDOS versions.
#include <osbind.h>
#include <stdio.h>

static long tos_version(void) {
  // The OS header, at _sysbase (0x4f2), has the TOS version at offset 2.
  // Low memory can only be read in supervisor mode.
  return *(unsigned short *)(*(char **)0x4f2 + 2);
}

int main(int argc, char **argv) {
  unsigned tos = (unsigned)Supexec(tos_version);
  unsigned gemdos = Sversion();
  printf("Hello from clang on TOS %x.%02x (GEMDOS %d.%02d)!\n", tos >> 8,
         tos & 0xff, gemdos & 0xff, gemdos >> 8);
  for (int i = 1; i < argc; ++i)
    printf("arg %d: %s\n", i, argv[i]);
  return 0;
}
