#ifndef TIM_H
#define TIM_H

static struct timeval tim_tv = {0};

static void tim_gettime(struct timeval * tv) {
#ifdef _WIN32
  SYSTEMTIME st; GetSystemTime(&st);

  // Contains a 64-bit value representing the number of 100-nanosecond
  // intervals since January 1, 1601 (UTC).
  FILETIME ft; assert(SystemTimeToFileTime(&st, &ft));

  ULARGE_INTEGER i;
  i.u.LowPart  = ft.dwLowDateTime;
  i.u.HighPart = ft.dwHighDateTime;

  ULONGLONG usec = i.QuadPart / 10;
  tv->tv_sec  = usec / (1000*1000);
  tv->tv_usec = usec % (1000*1000);
#else
  gettimeofday(tv, NULL);
#endif
}

static float tim_now() {
  if (tim_tv.tv_sec == 0) {
    tim_gettime(&tim_tv);
    return 0.0;
  }

  struct timeval tv;
  tim_gettime(&tv);
  float secs = (tv.tv_sec - tim_tv.tv_sec);
  float usecs = (tv.tv_usec - tim_tv.tv_usec);
  return secs + usecs / 1000000.f;
}

#endif

