// Checks the code generator and the C library on TOS: calls with mixed
// argument sizes, struct return, 64-bit and floating point arithmetic,
// varargs, setjmp, static constructors, malloc, files, directories and
// time. Prints one line per failure and a summary.
#define _GNU_SOURCE
#include <errno.h>
#include <osbind.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static int failures, checks;
#define CHECK(c)                                                               \
  do {                                                                         \
    ++checks;                                                                  \
    if (!(c)) {                                                                \
      ++failures;                                                              \
      printf("FAIL line %d: %s\n", __LINE__, #c);                              \
    }                                                                          \
  } while (0)

struct big { long a, b, c; short d; char e; };

__attribute__((noinline)) static struct big make_big(long x, short y, char z) {
  struct big b = {x, x * 2, x * 3, y, z};
  return b;
}

__attribute__((noinline)) static long mix(char a, short b, long c, long long d,
                                          double e, float f) {
  return a + b + c + (long)(d >> 32) + (long)e + (long)f;
}

static long sum(int n, ...) {
  va_list ap;
  long s = 0;
  va_start(ap, n);
  for (int i = 0; i < n; ++i)
    s += va_arg(ap, int);
  va_end(ap);
  return s;
}

static int ctor_ran;
__attribute__((constructor)) static void ctor(void) { ctor_ran = 42; }

static jmp_buf jb;
__attribute__((noinline)) static void jump(int v) { longjmp(jb, v); }

volatile long long v64 = 0x123456789abcdefLL;
volatile double vd = 2.5;
long data_word = 0x12345678;
char bss_buf[1000];

int main(void) {
  // Code generation.
  struct big b = make_big(1000, -5, 'x');
  CHECK(b.a == 1000 && b.b == 2000 && b.c == 3000 && b.d == -5 && b.e == 'x');
  CHECK(mix(1, 2, 3, 4LL << 32, 5.9, 6.5f) == 21);
  CHECK(sum(4, 10, 20, 30, 40) == 100);
  CHECK(v64 / 1000 == 81985529216486LL && v64 % 1000 == 895);
  CHECK((unsigned long long)v64 >> 40 == 0x12345);
  CHECK(vd * 4 == 10.0 && (int)(vd * vd * 100) == 625);
  char fbuf[32];
  snprintf(fbuf, sizeof fbuf, "%.3f %g %e", 3.14159, 0.5, 12345.0);
  CHECK(!strcmp(fbuf, "3.142 0.5 1.234500e+04"));
  CHECK(ctor_ran == 42);
  CHECK(data_word == 0x12345678);
  int zero = 1;
  for (unsigned i = 0; i < sizeof bss_buf; ++i)
    zero &= bss_buf[i] == 0;
  CHECK(zero);
  int j = setjmp(jb);
  if (!j)
    jump(7);
  CHECK(j == 7);

  // malloc.
  char *p = malloc(100000);
  CHECK(p != NULL);
  if (p) {
    memset(p, 0x5a, 100000);
    p = realloc(p, 200000);
    CHECK(p && p[99999] == 0x5a);
    free(p);
  }

  // Files.
  FILE *f = fopen("C:\\ABITEST.TXT", "w");
  CHECK(f != NULL);
  if (f) {
    fprintf(f, "line %d\nsecond line\n", 1);
    fclose(f);
  }
  f = fopen("C:/abitest.txt", "a");
  CHECK(f != NULL);
  if (f) {
    fputs("third\n", f);
    fclose(f);
  }
  struct stat st;
  CHECK(stat("C:\\ABITEST.TXT", &st) == 0 && st.st_size == 25 &&
        S_ISREG(st.st_mode));
  f = fopen("C:\\ABITEST.TXT", "r");
  char line[64] = "";
  CHECK(f && fgets(line, sizeof line, f) && !strcmp(line, "line 1\n"));
  CHECK(f && fgets(line, sizeof line, f) && !strcmp(line, "second line\n"));
  CHECK(f && fgets(line, sizeof line, f) && !strcmp(line, "third\n"));
  CHECK(f && !fgets(line, sizeof line, f) && feof(f));
  if (f) {
    fseek(f, 5, SEEK_SET);
    CHECK(fgetc(f) == '1' && ftell(f) == 6);
    fclose(f);
  }
  CHECK(fopen("C:\\NOSUCH.TXT", "r") == NULL && errno == ENOENT);
  CHECK(mkdir("C:\\ABIDIR", 0777) == 0);
  CHECK(stat("C:\\ABIDIR", &st) == 0 && S_ISDIR(st.st_mode));
  CHECK(rename("C:\\ABITEST.TXT", "C:\\ABIDIR\\MOVED.TXT") == 0);
  CHECK(chdir("C:\\ABIDIR") == 0);
  char cwd[64];
  CHECK(getcwd(cwd, sizeof cwd) && !strcmp(cwd, "C:\\ABIDIR"));
  CHECK(access("MOVED.TXT", R_OK) == 0);
  CHECK(remove("MOVED.TXT") == 0);
  CHECK(chdir("C:\\") == 0);
  CHECK(rmdir("C:\\ABIDIR") == 0);
  CHECK(stat("C:\\ABIDIR", &st) == -1 && errno == ENOENT);

  // Time.
  time_t t = time(NULL);
  CHECK(t > 315532800); // after 1980
  struct timespec a, c;
  clock_gettime(CLOCK_MONOTONIC, &a);
  struct timespec d = {0, 100000000};
  nanosleep(&d, NULL);
  clock_gettime(CLOCK_MONOTONIC, &c);
  long ms = (c.tv_sec - a.tv_sec) * 1000 + (c.tv_nsec - a.tv_nsec) / 1000000;
  CHECK(ms >= 95 && ms < 1000);

  // isatty: the console is a terminal.
  CHECK(isatty(1) && isatty(2));

  fprintf(stderr, "stderr works\n");
  printf("abitest: %d checks, %d failures\n", checks, failures);
  return failures != 0;
}
