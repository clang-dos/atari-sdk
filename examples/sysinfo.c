// Shows what the GEMDOS, BIOS and XBIOS calls in <osbind.h> report about
// the machine, without the C library's stdio: everything goes through
// Cconws().
#include <osbind.h>

static void put(const char *s) { Cconws(s); }

static void putnum(unsigned long n, int base) {
  char buf[12], *p = buf + sizeof(buf) - 1;
  *p = '\0';
  do {
    *--p = "0123456789abcdef"[n % base];
    n /= base;
  } while (n);
  put(p);
}

static long cookie_jar(void) { return *(long *)0x5a0; }

static long find_cookie(unsigned long id) {
  long *jar = (long *)Supexec(cookie_jar);
  for (; jar && jar[0]; jar += 2)
    if ((unsigned long)jar[0] == id)
      return jar[1];
  return -1;
}

int main(void) {
  static const char *rez[] = {"320x200, 16 colours", "640x200, 4 colours",
                              "640x400, mono"};
  short r = Getrez();
  put("Screen: ");
  put(r >= 0 && r <= 2 ? rez[r] : "other");
  put("\r\nDrives:");
  unsigned long map = Drvmap();
  for (int d = 0; d < 26; ++d)
    if (map & (1UL << d)) {
      char s[3] = {' ', (char)('A' + d), 0};
      put(s);
    }
  put("\r\nFree memory: ");
  putnum((unsigned long)Malloc(-1L), 10);
  put(" bytes\r\nCPU: 680");
  long cpu = find_cookie(0x5f435055); // _CPU
  putnum(cpu < 0 ? 0 : cpu, 10);
  long mch = find_cookie(0x5f4d4348); // _MCH
  static const char *mchs[] = {"ST", "STe", "TT", "Falcon"};
  put("\r\nMachine: ");
  put(mch < 0 ? "ST (no cookie jar)" : (mch >> 16) <= 3 ? mchs[mch >> 16] : "other");
  unsigned short date = Tgetdate(), time = Tgettime();
  put("\r\nDate: ");
  putnum(1980 + (date >> 9), 10);
  put("-");
  putnum((date >> 5) & 15, 10);
  put("-");
  putnum(date & 31, 10);
  put(" ");
  putnum(time >> 11, 10);
  put(":");
  putnum((time >> 5) & 63, 10);
  put("\r\nsysinfo done\r\n");
  return 0;
}
