// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"


#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <time.h>


// 以下代码是为测试,摘自己Linux的Glibc.
// #define ESAY 

//////////////////////////////////////////////////////////////////////////

int
strncasecmp (const char *s1, const char *s2, size_t n)
{
	register const unsigned char *p1 = (const unsigned char *) s1;
	register const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2 || n == 0)
		return 0;

	do
	{
		c1 = tolower (*p1++);
		c2 = tolower (*p2++);
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
	} while (--n > 0);

	return c1 - c2;
}


#ifndef ESAY



#if ! HAVE_LOCALTIME_R && ! defined localtime_r
# ifdef _LIBC
#  define localtime_r __localtime_r
# else
/* Approximate localtime_r as best we can in its absence.  */
#  define localtime_r my_localtime_r
static struct tm *localtime_r (const time_t *, struct tm *);
static struct tm *
localtime_r (const time_t* t, struct tm *tp)
{
  struct tm *l = localtime (t);
  if (! l)
    return 0;
  *tp = *l;
  return tp;
}
# endif /* ! _LIBC */
#endif /* ! HAVE_LOCALTIME_R && ! defined (localtime_r) */


#define match_char(ch1, ch2) if (ch1 != ch2) return NULL
#if defined __GNUC__ && __GNUC__ >= 2
# define match_string(cs1, s2) \
  ({ size_t len = strlen (cs1);                                               \
     int result = strncasecmp ((cs1), (s2), len) == 0;                        \
     if (result) (s2) += len;                                                 \
     result; })
#else
/* Oh come on.  Get a reasonable compiler.  */
# define match_string(cs1, s2) \
  (strncasecmp ((cs1), (s2), strlen (cs1)) ? 0 : ((s2) += strlen (cs1), 1))
#endif
/* We intentionally do not use isdigit() for testing because this will
   lead to problems with the wide character version.  */
#define get_number(from, to, n) \
  do {                                                                        \
    int __n = n;                                                              \
    val = 0;                                                                  \
    while (*rp == ' ')                                                        \
      ++rp;                                                                   \
    if (*rp < '0' || *rp > '9')                                               \
      return NULL;                                                            \
    do {                                                                      \
      val *= 10;                                                              \
      val += *rp++ - '0';                                                     \
    } while (--__n > 0 && val * 10 <= to && *rp >= '0' && *rp <= '9');        \
    if (val < from || val > to)                                               \
      return NULL;                                                            \
  } while (0)
#ifdef _NL_CURRENT
# define get_alt_number(from, to, n) \
  ({                                                                          \
    __label__ do_normal;                                                      \
    if (*decided != raw)                                                      \
      {                                                                       \
        const char *alts = _NL_CURRENT (LC_TIME, ALT_DIGITS);                 \
        int __n = n;                                                          \
        int any = 0;                                                          \
        while (*rp == ' ')                                                    \
          ++rp;                                                               \
        val = 0;                                                              \
        do {                                                                  \
          val *= 10;                                                          \
          while (*alts != '\0')                                               \
            {                                                                 \
              size_t len = strlen (alts);                                     \
              if (strncasecmp (alts, rp, len) == 0)                           \
                break;                                                        \
              alts += len + 1;                                                \
              ++val;                                                          \
            }                                                                 \
          if (*alts == '\0')                                                  \
            {                                                                 \
              if (*decided == not && ! any)                                   \
                goto do_normal;                                               \
              /* If we haven't read anything it's an error.  */               \
              if (! any)                                                      \
                return NULL;                                                  \
              /* Correct the premature multiplication.  */                    \
              val /= 10;                                                      \
              break;                                                          \
            }                                                                 \
          else                                                                \
            *decided = loc;                                                   \
        } while (--__n > 0 && val * 10 <= to);                                \
        if (val < from || val > to)                                           \
          return NULL;                                                        \
      }                                                                       \
    else                                                                      \
      {                                                                       \
       do_normal:                                                             \
        get_number (from, to, n);                                             \
      }                                                                       \
    0;                                                                        \
  })
#else
# define get_alt_number(from, to, n) \
  /* We don't have the alternate representation.  */                          \
  get_number(from, to, n)
#endif
#define recursive(new_fmt) \
  (*(new_fmt) != '\0'                                                         \
   && (rp = strptime_internal (rp, (new_fmt), tm, decided, era_cnt)) != NULL)


#ifdef _LIBC
/* This is defined in locale/C-time.c in the GNU libc.  */
extern const struct locale_data _nl_C_LC_TIME;
extern const unsigned short int __mon_yday[2][13];

# define weekday_name (&_nl_C_LC_TIME.values[_NL_ITEM_INDEX (DAY_1)].string)
# define ab_weekday_name \
  (&_nl_C_LC_TIME.values[_NL_ITEM_INDEX (ABDAY_1)].string)
# define month_name (&_nl_C_LC_TIME.values[_NL_ITEM_INDEX (MON_1)].string)
# define ab_month_name (&_nl_C_LC_TIME.values[_NL_ITEM_INDEX (ABMON_1)].string)
# define HERE_D_T_FMT (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (D_T_FMT)].string)
# define HERE_D_FMT (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (D_FMT)].string)
# define HERE_AM_STR (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (AM_STR)].string)
# define HERE_PM_STR (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (PM_STR)].string)
# define HERE_T_FMT_AMPM \
  (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (T_FMT_AMPM)].string)
# define HERE_T_FMT (_nl_C_LC_TIME.values[_NL_ITEM_INDEX (T_FMT)].string)

# define strncasecmp(s1, s2, n) __strncasecmp (s1, s2, n)
#else
static char const weekday_name[][10] =
  {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };
static char const ab_weekday_name[][4] =
  {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
static char const month_name[][10] =
  {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
  };
static char const ab_month_name[][4] =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
# define HERE_D_T_FMT "%a %b %e %H:%M:%S %Y"
# define HERE_D_FMT "%m/%d/%y"
# define HERE_AM_STR "AM"
# define HERE_PM_STR "PM"
# define HERE_T_FMT_AMPM "%I:%M:%S %p"
# define HERE_T_FMT "%H:%M:%S"

const unsigned short int __mon_yday[2][13] =
  {
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
  };
#endif

/* Status of lookup: do we use the locale data or the raw data?  */
enum locale_status { not, loc, raw };


#ifndef __isleap
/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is).  */
# define __isleap(year) \
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#endif

/* Compute the day of the week.  */
static void
day_of_the_week (struct tm *tm)
{
  /* We know that January 1st 1970 was a Thursday (= 4).  Compute the
     the difference between this data in the one on TM and so determine
     the weekday.  */
  int corr_year = 1900 + tm->tm_year - (tm->tm_mon < 2);
  int wday = (-473
              + (365 * (tm->tm_year - 70))
              + (corr_year / 4)
              - ((corr_year / 4) / 25) + ((corr_year / 4) % 25 < 0)
              + (((corr_year / 4) / 25) / 4)
              + __mon_yday[0][tm->tm_mon]
              + tm->tm_mday - 1);
  tm->tm_wday = ((wday % 7) + 7) % 7;
}

/* Compute the day of the year.  */
static void
day_of_the_year (struct tm *tm)
{
  tm->tm_yday = (__mon_yday[__isleap (1900 + tm->tm_year)][tm->tm_mon]
                 + (tm->tm_mday - 1));
}

static char *
#ifdef _LIBC
internal_function
#endif
strptime_internal (const char *rp, const char *fmt, struct tm *tm,
                        enum locale_status *decided, int era_cnt);

static char *
#ifdef _LIBC
internal_function
#endif
strptime_internal (const char *rp, const char *fmt,struct tm * tm, enum locale_status *decided, int era_cnt)
{
  const char *rp_backup;
  int cnt;
  size_t val;
  int have_I, is_pm;
  int century, want_century;
  int want_era;
  int have_wday, want_xday;
  int have_yday;
  int have_mon, have_mday;
#ifdef _NL_CURRENT
  size_t num_eras;
#endif
  struct era_entry *era;

  have_I = is_pm = 0;
  century = -1;
  want_century = 0;
  want_era = 0;
  era = NULL;

  have_wday = want_xday = have_yday = have_mon = have_mday = 0;

  while (*fmt != '\0')
    {
      /* A white space in the format string matches 0 more or white
         space in the input string.  */
      if (isspace (*fmt))
        {
          while (isspace (*rp))
            ++rp;
          ++fmt;
          continue;
        }

      /* Any character but `%' must be matched by the same character
         in the iput string.  */
      if (*fmt != '%')
        {
          match_char (*fmt++, *rp++);
          continue;
        }

      ++fmt;
#ifndef _NL_CURRENT
      /* We need this for handling the `E' modifier.  */
    start_over:
#endif

      /* Make back up of current processing pointer.  */
      rp_backup = rp;

      switch (*fmt++)
        {
        case '%':
          /* Match the `%' character itself.  */
          match_char ('%', *rp++);
          break;
        case 'a':
        case 'A':
          /* Match day of week.  */
          for (cnt = 0; cnt < 7; ++cnt)
            {
#ifdef _NL_CURRENT
              if (*decided !=raw)
                {
                  if (match_string (_NL_CURRENT (LC_TIME, DAY_1 + cnt), rp))
                    {
                      if (*decided == not
                          && strcmp (_NL_CURRENT (LC_TIME, DAY_1 + cnt),
                                     weekday_name[cnt]))
                        *decided = loc;
                      break;
                    }
                  if (match_string (_NL_CURRENT (LC_TIME, ABDAY_1 + cnt), rp))
                    {
                      if (*decided == not
                          && strcmp (_NL_CURRENT (LC_TIME, ABDAY_1 + cnt),
                                     ab_weekday_name[cnt]))
                        *decided = loc;
                      break;
                    }
                }
#endif
              if (*decided != loc
                  && (match_string (weekday_name[cnt], rp)
                      || match_string (ab_weekday_name[cnt], rp)))
                {
                  *decided = raw;
                  break;
                }
            }
          if (cnt == 7)
            /* Does not match a weekday name.  */
            return NULL;
          tm->tm_wday = cnt;
          have_wday = 1;
          break;
        case 'b':
        case 'B':
        case 'h':
          /* Match month name.  */
          for (cnt = 0; cnt < 12; ++cnt)
            {
#ifdef _NL_CURRENT
              if (*decided !=raw)
                {
                  if (match_string (_NL_CURRENT (LC_TIME, MON_1 + cnt), rp))
                    {
                      if (*decided == not
                          && strcmp (_NL_CURRENT (LC_TIME, MON_1 + cnt),
                                     month_name[cnt]))
                        *decided = loc;
                      break;
                    }
                  if (match_string (_NL_CURRENT (LC_TIME, ABMON_1 + cnt), rp))
                    {
                      if (*decided == not
                          && strcmp (_NL_CURRENT (LC_TIME, ABMON_1 + cnt),
                                     ab_month_name[cnt]))
                        *decided = loc;
                      break;
                    }
                }
#endif
              if (match_string (month_name[cnt], rp)
                  || match_string (ab_month_name[cnt], rp))
                {
                  *decided = raw;
                  break;
                }
            }
          if (cnt == 12)
            /* Does not match a month name.  */
            return NULL;
          tm->tm_mon = cnt;
          want_xday = 1;
          break;
        case 'c':
          /* Match locale's date and time format.  */
#ifdef _NL_CURRENT
          if (*decided != raw)
            {
              if (!recursive (_NL_CURRENT (LC_TIME, D_T_FMT)))
                {
                  if (*decided == loc)
                    return NULL;
                  else
                    rp = rp_backup;
                }
              else
                {
                  if (*decided == not &&
                      strcmp (_NL_CURRENT (LC_TIME, D_T_FMT), HERE_D_T_FMT))
                    *decided = loc;
                  want_xday = 1;
                  break;
                }
              *decided = raw;
            }
#endif
          if (!recursive (HERE_D_T_FMT))
            return NULL;
          want_xday = 1;
          break;
        case 'C':
          /* Match century number.  */
#ifdef _NL_CURRENT
        match_century:
#endif
          get_number (0, 99, 2);
          century = val;
          want_xday = 1;
          break;
        case 'd':
        case 'e':
          /* Match day of month.  */
          get_number (1, 31, 2);
          tm->tm_mday = val;
          have_mday = 1;
          want_xday = 1;
          break;
        case 'F':
          if (!recursive ("%Y-%m-%d"))
            return NULL;
          want_xday = 1;
          break;
        case 'x':
#ifdef _NL_CURRENT
          if (*decided != raw)
            {
              if (!recursive (_NL_CURRENT (LC_TIME, D_FMT)))
                {
                  if (*decided == loc)
                    return NULL;
                  else
                    rp = rp_backup;
                }
              else
                {
                  if (*decided == not
                      && strcmp (_NL_CURRENT (LC_TIME, D_FMT), HERE_D_FMT))
                    *decided = loc;
                  want_xday = 1;
                  break;
                }
              *decided = raw;
            }
#endif
          /* Fall through.  */
        case 'D':
          /* Match standard day format.  */
          if (!recursive (HERE_D_FMT))
            return NULL;
          want_xday = 1;
          break;
        case 'k':
        case 'H':
          /* Match hour in 24-hour clock.  */
          get_number (0, 23, 2);
          tm->tm_hour = val;
          have_I = 0;
          break;
        case 'I':
          /* Match hour in 12-hour clock.  */
          get_number (1, 12, 2);
          tm->tm_hour = val % 12;
          have_I = 1;
          break;
        case 'j':
          /* Match day number of year.  */
          get_number (1, 366, 3);
          tm->tm_yday = val - 1;
          have_yday = 1;
          break;
        case 'm':
          /* Match number of month.  */
          get_number (1, 12, 2);
          tm->tm_mon = val - 1;
          have_mon = 1;
          want_xday = 1;
          break;
        case 'M':
          /* Match minute.  */
          get_number (0, 59, 2);
          tm->tm_min = val;
          break;
        case 'n':
        case 't':
          /* Match any white space.  */
          while (isspace (*rp))
            ++rp;
          break;
        case 'p':
          /* Match locale's equivalent of AM/PM.  */
#ifdef _NL_CURRENT
          if (*decided != raw)
            {
              if (match_string (_NL_CURRENT (LC_TIME, AM_STR), rp))
                {
                  if (strcmp (_NL_CURRENT (LC_TIME, AM_STR), HERE_AM_STR))
                    *decided = loc;
                  break;
                }
              if (match_string (_NL_CURRENT (LC_TIME, PM_STR), rp))
                {
                  if (strcmp (_NL_CURRENT (LC_TIME, PM_STR), HERE_PM_STR))
                    *decided = loc;
                  is_pm = 1;
                  break;
                }
              *decided = raw;
            }
#endif
          if (!match_string (HERE_AM_STR, rp))
            if (match_string (HERE_PM_STR, rp))
              is_pm = 1;
            else
              return NULL;
          break;
        case 'r':
#ifdef _NL_CURRENT
          if (*decided != raw)
            {
              if (!recursive (_NL_CURRENT (LC_TIME, T_FMT_AMPM)))
                {
                  if (*decided == loc)
                    return NULL;
                  else
                    rp = rp_backup;
                }
              else
                {
                  if (*decided == not &&
                      strcmp (_NL_CURRENT (LC_TIME, T_FMT_AMPM),
                              HERE_T_FMT_AMPM))
                    *decided = loc;
                  break;
                }
              *decided = raw;
            }
#endif
          if (!recursive (HERE_T_FMT_AMPM))
            return NULL;
          break;
        case 'R':
          if (!recursive ("%H:%M"))
            return NULL;
          break;
        case 's':
          {
            /* The number of seconds may be very high so we cannot use
               the `get_number' macro.  Instead read the number
               character for character and construct the result while
               doing this.  */
            time_t secs = 0;
            if (*rp < '0' || *rp > '9')
              /* We need at least one digit.  */
              return NULL;

            do
              {
                secs *= 10;
                secs += *rp++ - '0';
              }
            while (*rp >= '0' && *rp <= '9');

            if (localtime_r (&secs, tm) == NULL)
              /* Error in function.  */
              return NULL;
          }
          break;
        case 'S':
          get_number (0, 61, 2);
          tm->tm_sec = val;
          break;
        case 'X':
#ifdef _NL_CURRENT
          if (*decided != raw)
            {
              if (!recursive (_NL_CURRENT (LC_TIME, T_FMT)))
                {
                  if (*decided == loc)
                    return NULL;
                  else
                    rp = rp_backup;
                }
              else
                {
                  if (strcmp (_NL_CURRENT (LC_TIME, T_FMT), HERE_T_FMT))
                    *decided = loc;
                  break;
                }
              *decided = raw;
            }
#endif
          /* Fall through.  */
        case 'T':
          if (!recursive (HERE_T_FMT))
            return NULL;
          break;
        case 'u':
          get_number (1, 7, 1);
          tm->tm_wday = val % 7;
          have_wday = 1;
          break;
        case 'g':
          get_number (0, 99, 2);
          /* XXX This cannot determine any field in TM.  */
          break;
        case 'G':
          if (*rp < '0' || *rp > '9')
            return NULL;
          /* XXX Ignore the number since we would need some more
             information to compute a real date.  */
          do
            ++rp;
          while (*rp >= '0' && *rp <= '9');
          break;
        case 'U':
        case 'V':
        case 'W':
          get_number (0, 53, 2);
          /* XXX This cannot determine any field in TM without some
             information.  */
          break;
        case 'w':
          /* Match number of weekday.  */
          get_number (0, 6, 1);
          tm->tm_wday = val;
          have_wday = 1;
          break;
        case 'y':
#ifdef _NL_CURRENT
        match_year_in_century:
#endif
          /* Match year within century.  */
          get_number (0, 99, 2);
          /* The "Year 2000: The Millennium Rollover" paper suggests that
             values in the range 69-99 refer to the twentieth century.  */
          tm->tm_year = val >= 69 ? val : val + 100;
          /* Indicate that we want to use the century, if specified.  */
          want_century = 1;
          want_xday = 1;
          break;
        case 'Y':
          /* Match year including century number.  */
          get_number (0, 9999, 4);
          tm->tm_year = val - 1900;
          want_century = 0;
          want_xday = 1;
          break;
        case 'Z':
          /* XXX How to handle this?  */
          break;
        case 'E':
#ifdef _NL_CURRENT
          switch (*fmt++)
            {
            case 'c':
              /* Match locale's alternate date and time format.  */
              if (*decided != raw)
                {
                  const char *fmt = _NL_CURRENT (LC_TIME, ERA_D_T_FMT);

                  if (*fmt == '\0')
                    fmt = _NL_CURRENT (LC_TIME, D_T_FMT);

                  if (!recursive (fmt))
                    {
                      if (*decided == loc)
                        return NULL;
                      else
                        rp = rp_backup;
                    }
                  else
                    {
                      if (strcmp (fmt, HERE_D_T_FMT))
                        *decided = loc;
                      want_xday = 1;
                      break;
                    }
                  *decided = raw;
                }
              /* The C locale has no era information, so use the
                 normal representation.  */
              if (!recursive (HERE_D_T_FMT))
                return NULL;
              want_xday = 1;
              break;
            case 'C':
              if (*decided != raw)
                {
                  if (era_cnt >= 0)
                    {
                      era = _nl_select_era_entry (era_cnt);
                      if (match_string (era->era_name, rp))
                        {
                          *decided = loc;
                          break;
                        }
                      else
                        return NULL;
                    }
                  else
                    {
                      num_eras = _NL_CURRENT_WORD (LC_TIME,
                                                   _NL_TIME_ERA_NUM_ENTRIES);
                      for (era_cnt = 0; era_cnt < (int) num_eras;
                           ++era_cnt, rp = rp_backup)
                        {
                          era = _nl_select_era_entry (era_cnt);
                          if (match_string (era->era_name, rp))
                            {
                              *decided = loc;
                              break;
                            }
                        }
                      if (era_cnt == (int) num_eras)
                        {
                          era_cnt = -1;
                          if (*decided == loc)
                            return NULL;
                        }
                      else
                        break;
                    }

                  *decided = raw;
                }
              /* The C locale has no era information, so use the
                 normal representation.  */
              goto match_century;
            case 'y':
              if (*decided == raw)
                goto match_year_in_century;

              get_number(0, 9999, 4);
              tm->tm_year = val;
              want_era = 1;
              want_xday = 1;
              break;
            case 'Y':
              if (*decided != raw)
                {
                  num_eras = _NL_CURRENT_WORD (LC_TIME,
                                               _NL_TIME_ERA_NUM_ENTRIES);
                  for (era_cnt = 0; era_cnt < (int) num_eras;
                       ++era_cnt, rp = rp_backup)
                    {
                      era = _nl_select_era_entry (era_cnt);
                      if (recursive (era->era_format))
                        break;
                    }
                  if (era_cnt == (int) num_eras)
                    {
                      era_cnt = -1;
                      if (*decided == loc)
                        return NULL;
                      else
                        rp = rp_backup;
                    }
                  else
                    {
                      *decided = loc;
                      era_cnt = -1;
                      break;
                    }

                  *decided = raw;
                }
              get_number (0, 9999, 4);
              tm->tm_year = val - 1900;
              want_century = 0;
              want_xday = 1;
              break;
            case 'x':
              if (*decided != raw)
                {
                  const char *fmt = _NL_CURRENT (LC_TIME, ERA_D_FMT);

                  if (*fmt == '\0')
                    fmt = _NL_CURRENT (LC_TIME, D_FMT);

                  if (!recursive (fmt))
                    {
                      if (*decided == loc)
                        return NULL;
                      else
                        rp = rp_backup;
                    }
                  else
                    {
                      if (strcmp (fmt, HERE_D_FMT))
                        *decided = loc;
                      break;
                    }
                  *decided = raw;
                }
              if (!recursive (HERE_D_FMT))
                return NULL;
              break;
            case 'X':
              if (*decided != raw)
                {
                  const char *fmt = _NL_CURRENT (LC_TIME, ERA_T_FMT);

                  if (*fmt == '\0')
                    fmt = _NL_CURRENT (LC_TIME, T_FMT);

                  if (!recursive (fmt))
                    {
                      if (*decided == loc)
                        return NULL;
                      else
                        rp = rp_backup;
                    }
                  else
                    {
                      if (strcmp (fmt, HERE_T_FMT))
                        *decided = loc;
                      break;
                    }
                  *decided = raw;
                }
              if (!recursive (HERE_T_FMT))
                return NULL;
              break;
            default:
              return NULL;
            }
          break;
#else
          /* We have no information about the era format.  Just use
             the normal format.  */
          if (*fmt != 'c' && *fmt != 'C' && *fmt != 'y' && *fmt != 'Y'
              && *fmt != 'x' && *fmt != 'X')
            /* This is an illegal format.  */
            return NULL;

          goto start_over;
#endif
        case 'O':
          switch (*fmt++)
            {
            case 'd':
            case 'e':
              /* Match day of month using alternate numeric symbols.  */
              get_alt_number (1, 31, 2);
              tm->tm_mday = val;
              have_mday = 1;
              want_xday = 1;
              break;
            case 'H':
              /* Match hour in 24-hour clock using alternate numeric
                 symbols.  */
              get_alt_number (0, 23, 2);
              tm->tm_hour = val;
              have_I = 0;
              break;
            case 'I':
              /* Match hour in 12-hour clock using alternate numeric
                 symbols.  */
              get_alt_number (1, 12, 2);
              tm->tm_hour = val - 1;
              have_I = 1;
              break;
            case 'm':
              /* Match month using alternate numeric symbols.  */
              get_alt_number (1, 12, 2);
              tm->tm_mon = val - 1;
              have_mon = 1;
              want_xday = 1;
              break;
            case 'M':
              /* Match minutes using alternate numeric symbols.  */
              get_alt_number (0, 59, 2);
              tm->tm_min = val;
              break;
            case 'S':
              /* Match seconds using alternate numeric symbols.  */
              get_alt_number (0, 61, 2);
              tm->tm_sec = val;
              break;
            case 'U':
            case 'V':
            case 'W':
              get_alt_number (0, 53, 2);
              /* XXX This cannot determine any field in TM without
                 further information.  */
              break;
            case 'w':
              /* Match number of weekday using alternate numeric symbols.  */
              get_alt_number (0, 6, 1);
              tm->tm_wday = val;
              have_wday = 1;
              break;
            case 'y':
              /* Match year within century using alternate numeric symbols.  */
              get_alt_number (0, 99, 2);
              tm->tm_year = val >= 69 ? val : val + 100;
              want_xday = 1;
              break;
            default:
              return NULL;
            }
          break;
        default:
          return NULL;
        }
    }

  if (have_I && is_pm)
    tm->tm_hour += 12;

  if (century != -1)
    {
      if (want_century)
        tm->tm_year = tm->tm_year % 100 + (century - 19) * 100;
      else
        /* Only the century, but not the year.  Strange, but so be it.  */
        tm->tm_year = (century - 19) * 100;
    }

#ifdef _NL_CURRENT
  if (era_cnt != -1)
    {
      era = _nl_select_era_entry(era_cnt);
      if (want_era)
        tm->tm_year = (era->start_date[0]
                       + ((tm->tm_year - era->offset)
                          * era->absolute_direction));
      else
        /* Era start year assumed.  */
        tm->tm_year = era->start_date[0];
    }
  else
#endif
    if (want_era)
      return NULL;

  if (want_xday && !have_wday)
    {
      if ( !(have_mon && have_mday) && have_yday)
        {
          /* We don't have tm_mon and/or tm_mday, compute them.  */
          int t_mon = 0;
          while (__mon_yday[__isleap(1900 + tm->tm_year)][t_mon] <= tm->tm_yday)
              t_mon++;
          if (!have_mon)
              tm->tm_mon = t_mon - 1;
          if (!have_mday)
              tm->tm_mday =
                (tm->tm_yday
                 - __mon_yday[__isleap(1900 + tm->tm_year)][t_mon - 1] + 1);
        }
      day_of_the_week (tm);
    }
  if (want_xday && !have_yday)
    day_of_the_year (tm);

  return (char *) rp;
}


char *
strptime (const char *buf, const char *format, struct tm *tm)
{
  enum locale_status decided;

#ifdef _NL_CURRENT
  decided = not;
#else
  decided = raw;
#endif
  return strptime_internal (buf, format, tm, &decided, -1);
}




#else

struct lc_time_T {
	const char *    mon[12];
	const char *    month[12];
	const char *    wday[7];
	const char *    weekday[7];
	const char *    X_fmt;
	const char *    x_fmt;
	const char *    c_fmt;
	const char *    am;
	const char *    pm;
	const char *    date_fmt;
	const char *    alt_month[12];
	const char *    Ef_fmt;
	const char *    EF_fmt;
};

struct lc_time_T _time_localebuf;
int _time_using_locale;
const struct lc_time_T _C_time_locale;

#define Locale  (_time_using_locale ? &_time_localebuf : &_C_time_locale)
static char * _strptime(const char *, const char *, struct tm *);

#ifdef  _THREAD_SAFE
static struct pthread_mutex     _gotgmt_mutexd = PTHREAD_MUTEX_STATIC_INITIALIZER;
static pthread_mutex_t          gotgmt_mutex   = &_gotgmt_mutexd;
#endif
static int got_GMT;

#define asizeof(a)      (sizeof (a) / sizeof ((a)[0]))

static char *
_strptime(const char *buf, const char *fmt, struct tm *tm)
{
        char    c;
        const char *ptr;
        int     i,
                len;
        int Ealternative, Oalternative;

        ptr = fmt;
        while (*ptr != 0) {
                if (*buf == 0)
                        break;

                c = *ptr++;

                if (c != '%') {
                        if (isspace((unsigned char)c))
                                while (*buf != 0 && isspace((unsigned char)*buf))
                                        buf++;
                        else if (c != *buf++)
                                return 0;
                        continue;
                }

                Ealternative = 0;
                Oalternative = 0;
label:
                c = *ptr++;
                switch (c) {
                case 0:
                case '%':
                        if (*buf++ != '%')
                                return 0;
                        break;

                case '+':
                        buf = _strptime(buf, Locale->date_fmt, tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'C':
                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        /* XXX This will break for 3-digit centuries. */
                        len = 2;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }
                        if (i < 19)
                                return 0;

                        tm->tm_year = i * 100 - 1900;
                        break;

                case 'c':
                        buf = _strptime(buf, Locale->c_fmt, tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'D':
                        buf = _strptime(buf, "%m/%d/%y", tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'E':
                        if (Ealternative || Oalternative)
                                break;
                        Ealternative++;
                        goto label;

                case 'O':
                        if (Ealternative || Oalternative)
                                break;
                        Oalternative++;
                        goto label;

                case 'F':
                case 'f':
                        if (!Ealternative)
                                break;
                        buf = _strptime(buf, (c == 'f') ? Locale->Ef_fmt : Locale->EF_fmt, tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'R':
                        buf = _strptime(buf, "%H:%M", tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'r':
                        buf = _strptime(buf, "%I:%M:%S %p", tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'T':
                        buf = _strptime(buf, "%H:%M:%S", tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'X':
                        buf = _strptime(buf, Locale->X_fmt, tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'x':
                        buf = _strptime(buf, Locale->x_fmt, tm);
                        if (buf == 0)
                                return 0;
                        break;

                case 'j':
                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        len = 3;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }
                        if (i < 1 || i > 366)
                                return 0;

                        tm->tm_yday = i - 1;
                        break;

                case 'M':
                case 'S':
                        if (*buf == 0 || isspace((unsigned char)*buf))
                                break;

                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        len = 2;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }

                        if (c == 'M') {
                                if (i > 59)
                                        return 0;
                                tm->tm_min = i;
                        } else {
                                if (i > 60)
                                        return 0;
                                tm->tm_sec = i;
                        }

                        if (*buf != 0 && isspace((unsigned char)*buf))
                                while (*ptr != 0 && !isspace((unsigned char)*ptr))
                                        ptr++;
                        break;

                case 'H':
                case 'I':
                case 'k':
                case 'l':
                        /*
                         * Of these, %l is the only specifier explicitly
                         * documented as not being zero-padded.  However,
                         * there is no harm in allowing zero-padding.
                         *
                         * XXX The %l specifier may gobble one too many
                         * digits if used incorrectly.
                         */
                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        len = 2;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }
                        if (c == 'H' || c == 'k') {
                                if (i > 23)
                                        return 0;
                        } else if (i > 12)
                                return 0;

                        tm->tm_hour = i;

                        if (*buf != 0 && isspace((unsigned char)*buf))
                                while (*ptr != 0 && !isspace((unsigned char)*ptr))
                                        ptr++;
                        break;

                case 'p':
                        /*
                         * XXX This is bogus if parsed before hour-related
                         * specifiers.
                         */
                        len = strlen(Locale->am);
                        if (strncasecmp(buf, Locale->am, len) == 0) {
                                if (tm->tm_hour > 12)
                                        return 0;
                                if (tm->tm_hour == 12)
                                        tm->tm_hour = 0;
                                buf += len;
                                break;
                        }

                        len = strlen(Locale->pm);
                        if (strncasecmp(buf, Locale->pm, len) == 0) {
                                if (tm->tm_hour > 12)
                                        return 0;
                                if (tm->tm_hour != 12)
                                        tm->tm_hour += 12;
                                buf += len;
                                break;
                        }

                        return 0;

                case 'A':
                case 'a':
                        for (i = 0; i < asizeof(Locale->weekday); i++) {
                                if (c == 'A') {
                                        len = strlen(Locale->weekday[i]);
                                        if (strncasecmp(buf,
                                                        Locale->weekday[i],
                                                        len) == 0)
                                                break;
                                } else {
                                        len = strlen(Locale->wday[i]);
                                        if (strncasecmp(buf,
                                                        Locale->wday[i],
                                                        len) == 0)
                                                break;
                                }
                        }
                        if (i == asizeof(Locale->weekday))
                                return 0;

                        tm->tm_wday = i;
                        buf += len;
                        break;

                case 'U':
                case 'W':
                        /*
                         * XXX This is bogus, as we can not assume any valid
                         * information present in the tm structure at this
                         * point to calculate a real value, so just check the
                         * range for now.
                         */
                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        len = 2;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }
                        if (i > 53)
                                return 0;

                        if (*buf != 0 && isspace((unsigned char)*buf))
                                while (*ptr != 0 && !isspace((unsigned char)*ptr))
                                        ptr++;
                        break;

                case 'w':
                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        i = *buf - '0';
                        if (i > 6)
                                return 0;

                        tm->tm_wday = i;

                        if (*buf != 0 && isspace((unsigned char)*buf))
                                while (*ptr != 0 && !isspace((unsigned char)*ptr))
                                        ptr++;
                        break;

                case 'd':
                case 'e':
                        /*
                         * The %e specifier is explicitly documented as not
                         * being zero-padded but there is no harm in allowing
                         * such padding.
                         *
                         * XXX The %e specifier may gobble one too many
                         * digits if used incorrectly.
                         */
                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        len = 2;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }
                        if (i > 31)
                                return 0;

                        tm->tm_mday = i;

                        if (*buf != 0 && isspace((unsigned char)*buf))
                                while (*ptr != 0 && !isspace((unsigned char)*ptr))
                                        ptr++;
                        break;

                case 'B':
                case 'b':
                case 'h':
                        for (i = 0; i < asizeof(Locale->month); i++) {
                                if (Oalternative) {
                                        if (c == 'B') {
                                                len = strlen(Locale->alt_month[i]);
                                                if (strncasecmp(buf,
                                                                Locale->alt_month[i],
                                                                len) == 0)
                                                        break;
                                        }
                                } else {
                                        if (c == 'B') {
                                                len = strlen(Locale->month[i]);
                                                if (strncasecmp(buf,
                                                                Locale->month[i],
                                                                len) == 0)
                                                        break;
                                        } else {
                                                len = strlen(Locale->mon[i]);
                                                if (strncasecmp(buf,
                                                                Locale->mon[i],
                                                                len) == 0)
                                                        break;
                                        }
                                }
                        }
                        if (i == asizeof(Locale->month))
                                return 0;

                        tm->tm_mon = i;
                        buf += len;
                        break;

                case 'm':
                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        len = 2;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }
                        if (i < 1 || i > 12)
                                return 0;

                        tm->tm_mon = i - 1;

                        if (*buf != 0 && isspace((unsigned char)*buf))
                                while (*ptr != 0 && !isspace((unsigned char)*ptr))
                                        ptr++;
                        break;

                case 'Y':
                case 'y':
                        if (*buf == 0 || isspace((unsigned char)*buf))
                                break;

                        if (!isdigit((unsigned char)*buf))
                                return 0;

                        len = (c == 'Y') ? 4 : 2;
                        for (i = 0; len && *buf != 0 && isdigit((unsigned char)*buf); buf++) {
                                i *= 10;
                                i += *buf - '0';
                                len--;
                        }
                        if (c == 'Y')
                                i -= 1900;
                        if (c == 'y' && i < 69)
                                i += 100;
                        if (i < 0)
                                return 0;

                        tm->tm_year = i;

                        if (*buf != 0 && isspace((unsigned char)*buf))
                                while (*ptr != 0 && !isspace((unsigned char)*ptr))
                                        ptr++;
                        break;

                case 'Z':
                        {
                        const char *cp;
                        char *zonestr;

                        for (cp = buf; *cp && isupper((unsigned char)*cp); ++cp) {/*empty*/}
                        if (cp - buf) {
                                zonestr = (char *)alloca( cp - buf + 1);
                                strncpy(zonestr, buf, cp - buf);
                                zonestr[cp - buf] = '\0';
                                tzset();
                                if (0 == strcmp(zonestr, "GMT")) {
                                    got_GMT = 1;
                                } else if (0 == strcmp(zonestr, tzname[0])) {
                                    tm->tm_isdst = 0;
                                } else if (0 == strcmp(zonestr, tzname[1])) {
                                    tm->tm_isdst = 1;
                                } else {
                                    return 0;
                                }
                                buf += cp - buf;
                        }
                        }
                        break;
                }
        }
        return (char *)buf;
}


char *
strptime(const char *buf, const char *fmt, struct tm *tm)
{
        char *ret;

#ifdef  _THREAD_SAFE
        pthread_mutex_lock(&gotgmt_mutex);
#endif

        got_GMT = 0;
        ret = _strptime(buf, fmt, tm);
        if (ret && got_GMT) {
//                 time_t t = timegm(tm);
//             localtime_r(&t, tm);
//                 got_GMT = 0;
        }

#ifdef  _THREAD_SAFE
        pthread_mutex_unlock(&gotgmt_mutex);
#endif

        return ret;
}
#endif



//////////////////////////////////////////////////////////////////////////

/* 
   The JitterBug bug tracking system
   a simple vslprintf for systems that don't have vsnprintf

   Copyright (C) Andrew Tridgell 1997

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// #include "jitterbug.h"

#ifndef HAVE_VSNPRINTF
static FILE *nullf;
#endif


char *mon_full_names[] = {
	"January", "February", "March", "April",
	"May", "June", "July", "August",
	"September", "October", "November", "December"
};

char *mon_short_names[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

char *day_full_names[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

char *day_short_names[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

/* slprintf() and vslprintf() are much like snprintf() and vsnprintf() except that:

   1) they _always_ null terminate
   2) they work on systems that don't have vsnprintf
   3) attempts to write more than n characters (including the null) produce a fatal error
*/

int vslprintf(char *str, int n, char *format, va_list ap)
{
	int ret;

	ret = vsnprintf(str, n, format, ap);
	if (ret > n || ret < 0) {
		str[n] = 0;
		ATLTRACE("string overflow with format [%20.20s]", format);
	}
	str[ret] = 0;
	return ret;
}


int slprintf(char *str, int n, char *format, ...)
{
	va_list ap;  
	int ret;

	va_start(ap, format);
	ret = vslprintf(str,n,format,ap);
	va_end(ap);
	return ret;
}

char *php_std_date(time_t t)
{
	struct tm *tm1;
	char *str;

	tm1 = gmtime(&t);
	str = (char *)malloc(81);
	str[0] = '\0';

	if (!tm1) {
		return str;
	}

	sprintf_s(str, 80, "%s, %02d %s %04d %02d:%02d:%02d GMT",
			day_short_names[tm1->tm_wday],
			tm1->tm_mday,
			mon_short_names[tm1->tm_mon],
			tm1->tm_year + 1900,
			tm1->tm_hour, tm1->tm_min, tm1->tm_sec);

	str[79] = 0;
	return (str);
}

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	time_t lt;
	
	lt = time(NULL);
	std::string str = php_std_date(lt);
	OutputDebugStringA(str.c_str());
	struct tm ts;
	// setlocale(LC_ALL, "chs");
	strptime(str.c_str(), "%a, %d %b %Y %T", &ts);

	{
		char buf[1024];
		struct tm* t;
		time_t T=time(0);
		t=localtime(&T);

		strftime(buf,sizeof(buf),"%c",t);
// 		str = strptime(buf,"%c",t) + std::string("\n");
// 		OutputDebugStringA(str.c_str());

		str = strptime("Tue, 31 May 2005 14:16:16 GMT","%a, %d %b %Y %T",t) + std::string("\n");
		OutputDebugStringA(str.c_str());
		sprintf(buf, "%2d.%02d.%d %2d:%02d:%02d\n",t->tm_mday,t->tm_mon+1,t->tm_year+1900,t->tm_hour,t->tm_min,t->tm_sec);
		OutputDebugStringA(buf);
	}

	CenterWindow(GetParent());
	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
