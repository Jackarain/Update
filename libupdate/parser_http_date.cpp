#include "updater_impl.hpp"

static const char * const Curl_wkday[] =
{"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

static const char * const weekday[] =
{ "Monday", "Tuesday", "Wednesday", "Thursday",
"Friday", "Saturday", "Sunday" };

static const char * const Curl_month[]=
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

typedef struct
{
   char name[5];
   int offset; /* +/- in minutes */
} tzinfo;

static int parsedate(const char *date, time_t *output);

#define PARSEDATE_OK     0
#define PARSEDATE_FAIL   -1
#define PARSEDATE_LATER  1
#define PARSEDATE_SOONER 2

#define tDAYZONE -60       /* offset for daylight savings time */
static const tzinfo tz[]= {
  {"GMT", 0},              /* Greenwich Mean */
  {"UTC", 0},              /* Universal (Coordinated) */
  {"WET", 0},              /* Western European */
  {"BST", 0 tDAYZONE},     /* British Summer */
  {"WAT", 60},             /* West Africa */
  {"AST", 240},            /* Atlantic Standard */
  {"ADT", 240 tDAYZONE},   /* Atlantic Daylight */
  {"EST", 300},            /* Eastern Standard */
  {"EDT", 300 tDAYZONE},   /* Eastern Daylight */
  {"CST", 360},            /* Central Standard */
  {"CDT", 360 tDAYZONE},   /* Central Daylight */
  {"MST", 420},            /* Mountain Standard */
  {"MDT", 420 tDAYZONE},   /* Mountain Daylight */
  {"PST", 480},            /* Pacific Standard */
  {"PDT", 480 tDAYZONE},   /* Pacific Daylight */
  {"YST", 540},            /* Yukon Standard */
  {"YDT", 540 tDAYZONE},   /* Yukon Daylight */
  {"HST", 600},            /* Hawaii Standard */
  {"HDT", 600 tDAYZONE},   /* Hawaii Daylight */
  {"CAT", 600},            /* Central Alaska */
  {"AHST", 600},           /* Alaska-Hawaii Standard */
  {"NT",  660},            /* Nome */
  {"IDLW", 720},           /* International Date Line West */
  {"CET", -60},            /* Central European */
  {"MET", -60},            /* Middle European */
  {"MEWT", -60},           /* Middle European Winter */
  {"MEST", -60 tDAYZONE},  /* Middle European Summer */
  {"CEST", -60 tDAYZONE},  /* Central European Summer */
  {"MESZ", -60 tDAYZONE},  /* Middle European Summer */
  {"FWT", -60},            /* French Winter */
  {"FST", -60 tDAYZONE},   /* French Summer */
  {"EET", -120},           /* Eastern Europe, USSR Zone 1 */
  {"WAST", -420},          /* West Australian Standard */
  {"WADT", -420 tDAYZONE}, /* West Australian Daylight */
  {"CCT", -480},           /* China Coast, USSR Zone 7 */
  {"JST", -540},           /* Japan Standard, USSR Zone 8 */
  {"EAST", -600},          /* Eastern Australian Standard */
  {"EADT", -600 tDAYZONE}, /* Eastern Australian Daylight */
  {"GST", -600},           /* Guam Standard, USSR Zone 9 */
  {"NZT", -720},           /* New Zealand */
  {"NZST", -720},          /* New Zealand Standard */
  {"NZDT", -720 tDAYZONE}, /* New Zealand Daylight */
  {"IDLE", -720},          /* International Date Line East */
  /* Next up: Military timezone names. RFC822 allowed these, but (as noted in
     RFC 1123) had their signs wrong. Here we use the correct signs to match
     actual military usage.
   */
  {"A",  +1 * 60},         /* Alpha */
  {"B",  +2 * 60},         /* Bravo */
  {"C",  +3 * 60},         /* Charlie */
  {"D",  +4 * 60},         /* Delta */
  {"E",  +5 * 60},         /* Echo */
  {"F",  +6 * 60},         /* Foxtrot */
  {"G",  +7 * 60},         /* Golf */
  {"H",  +8 * 60},         /* Hotel */
  {"I",  +9 * 60},         /* India */
  /* "J", Juliet is not used as a timezone, to indicate the observer's local
     time */
  {"K", +10 * 60},         /* Kilo */
  {"L", +11 * 60},         /* Lima */
  {"M", +12 * 60},         /* Mike */
  {"N",  -1 * 60},         /* November */
  {"O",  -2 * 60},         /* Oscar */
  {"P",  -3 * 60},         /* Papa */
  {"Q",  -4 * 60},         /* Quebec */
  {"R",  -5 * 60},         /* Romeo */
  {"S",  -6 * 60},         /* Sierra */
  {"T",  -7 * 60},         /* Tango */
  {"U",  -8 * 60},         /* Uniform */
  {"V",  -9 * 60},         /* Victor */
  {"W", -10 * 60},         /* Whiskey */
  {"X", -11 * 60},         /* X-ray */
  {"Y", -12 * 60},         /* Yankee */
  {"Z", 0},                /* Zulu, zero meridian, a.k.a. UTC */
};



/* Portable, consistent toupper (remember EBCDIC). Do not use toupper() because
   its behavior is altered by the current locale. */
static
char url_raw_toupper(char in)
{
  switch (in) {
  case 'a':
    return 'A';
  case 'b':
    return 'B';
  case 'c':
    return 'C';
  case 'd':
    return 'D';
  case 'e':
    return 'E';
  case 'f':
    return 'F';
  case 'g':
    return 'G';
  case 'h':
    return 'H';
  case 'i':
    return 'I';
  case 'j':
    return 'J';
  case 'k':
    return 'K';
  case 'l':
    return 'L';
  case 'm':
    return 'M';
  case 'n':
    return 'N';
  case 'o':
    return 'O';
  case 'p':
    return 'P';
  case 'q':
    return 'Q';
  case 'r':
    return 'R';
  case 's':
    return 'S';
  case 't':
    return 'T';
  case 'u':
    return 'U';
  case 'v':
    return 'V';
  case 'w':
    return 'W';
  case 'x':
    return 'X';
  case 'y':
    return 'Y';
  case 'z':
    return 'Z';
  }
  return in;
}

static
int url_raw_equal(const char *first, const char *second)
{
  while(*first && *second) {
    if(url_raw_toupper(*first) != url_raw_toupper(*second))
      /* get out of the loop as soon as they don't match */
      break;
    first++;
    second++;
  }
  /* we do the comparison here (possibly again), just to make sure that if the
     loop above is skipped because one of the strings reached zero, we must not
     return this as a successful match */
  return (url_raw_toupper(*first) == url_raw_toupper(*second));
}

int url_raw_nequal(const char *first, const char *second, size_t max)
{
   while(*first && *second && max) {
      if(url_raw_toupper(*first) != url_raw_toupper(*second)) {
         break;
      }
      max--;
      first++;
      second++;
   }
   if(0 == max)
      return 1; /* they are equal this far */

   return url_raw_toupper(*first) == url_raw_toupper(*second);
}

/* returns:
   -1 no day
   0 monday - 6 sunday
*/

static
int checkday(const char *check, size_t len)
{
  int i;
  const char * const *what;
  bool found = false;
  if(len > 3)
    what = &weekday[0];
  else
    what = &Curl_wkday[0];
  for(i = 0; i < 7; i++) {
    if(url_raw_equal(check, what[0])) {
      found = true;
      break;
    }
    what++;
  }
  return found ? i : -1;
}

static
int checkmonth(const char *check)
{
  int i;
  const char * const *what;
  bool found = false;

  what = &Curl_month[0];
  for(i = 0; i < 12; i++) {
    if(url_raw_equal(check, what[0])) {
      found = true;
      break;
    }
    what++;
  }
  return found ? i : -1; /* return the offset or -1, no real offset is -1 */
}

/* return the time zone offset between GMT and the input one, in number
   of seconds or -1 if the timezone wasn't found/legal */

static int checktz(const char *check)
{
  unsigned int i;
  const tzinfo *what;
  bool found = false;

  what = tz;
  for(i = 0; i < sizeof(tz) / sizeof(tz[0]); i++) {
    if(url_raw_equal(check, what->name)) {
      found = true;
      break;
    }
    what++;
  }
  return found ? what->offset * 60 : -1;
}

#ifndef ISALNUM
#define ISALNUM isalnum
#endif

#ifndef ISALPHA
#define ISALPHA isalpha
#endif

#ifndef ISDIGIT
#define ISDIGIT isdigit
#endif

#define CURL_MASK_SINT  0x7FFFFFFF

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  ifndef _USE_32BIT_TIME_T
#    define SIZEOF_TIME_T 8
#  else
#    define SIZEOF_TIME_T 4
#  endif
#endif

static 
int curlx_sltosi(long slnum)
{
   return (int)(slnum & (long) CURL_MASK_SINT);
}

static void skip(const char **date)
{
  /* skip everything that aren't letters or digits */
  while(**date && !ISALNUM(**date))
    (*date)++;
}

typedef enum {
  DATE_MDAY,
  DATE_YEAR,
  DATE_TIME
} assume;

/* this is a clone of 'struct tm' but with all fields we don't need or use
   cut out */
typedef struct {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
} my_tm;

/* struct tm to time since epoch in GMT time zone.
 * This is similar to the standard mktime function but for GMT only, and
 * doesn't suffer from the various bugs and portability problems that
 * some systems' implementations have.
 */
static
time_t my_timegm(my_tm *tm)
{
  static const int month_days_cumulative [12] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
  int month, year, leap_days;

  if(tm->tm_year < 70)
    /* we don't support years before 1970 as they will cause this function
       to return a negative value */
    return -1;

  year = tm->tm_year + 1900;
  month = tm->tm_mon;
  if(month < 0) {
    year += (11 - month) / 12;
    month = 11 - (11 - month) % 12;
  }
  else if(month >= 12) {
    year -= month / 12;
    month = month % 12;
  }

  leap_days = year - (tm->tm_mon <= 1);
  leap_days = ((leap_days / 4) - (leap_days / 100) + (leap_days / 400)
               - (1969 / 4) + (1969 / 100) - (1969 / 400));

  return ((((time_t) (year - 1970) * 365
            + leap_days + month_days_cumulative [month] + tm->tm_mday - 1) * 24
           + tm->tm_hour) * 60 + tm->tm_min) * 60 + tm->tm_sec;
}

static
int parsedate(const char *date, time_t *output)
{
  time_t t = 0;
  int wdaynum=-1;  /* day of the week number, 0-6 (mon-sun) */
  int monnum=-1;   /* month of the year number, 0-11 */
  int mdaynum=-1; /* day of month, 1 - 31 */
  int hournum=-1;
  int minnum=-1;
  int secnum=-1;
  int yearnum=-1;
  int tzoff=-1;
  my_tm tm;
  assume dignext = DATE_MDAY;
  const char *indate = date; /* save the original pointer */
  int part = 0; /* max 6 parts */

  while(*date && (part < 6)) {
    bool found=false;

    skip(&date);

    if(ISALPHA(*date)) {
      /* a name coming up */
      char buf[32]="";
      size_t len;
      sscanf(date, "%31[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]",
             buf);
      len = strlen(buf);

      if(wdaynum == -1) {
        wdaynum = checkday(buf, len);
        if(wdaynum != -1)
          found = true;
      }
      if(!found && (monnum == -1)) {
        monnum = checkmonth(buf);
        if(monnum != -1)
          found = true;
      }

      if(!found && (tzoff == -1)) {
        /* this just must be a time zone string */
        tzoff = checktz(buf);
        if(tzoff != -1)
          found = true;
      }

      if(!found)
        return PARSEDATE_FAIL; /* bad string */

      date += len;
    }
    else if(ISDIGIT(*date)) {
      /* a digit */
      int val;
      char *end;
      if((secnum == -1) &&
         (3 == sscanf(date, "%02d:%02d:%02d", &hournum, &minnum, &secnum))) {
        /* time stamp! */
        date += 8;
      }
      else if((secnum == -1) &&
              (2 == sscanf(date, "%02d:%02d", &hournum, &minnum))) {
        /* time stamp without seconds */
        date += 5;
        secnum = 0;
      }
      else {
        val = curlx_sltosi(strtol(date, &end, 10));

        if((tzoff == -1) &&
           ((end - date) == 4) &&
           (val <= 1400) &&
           (indate< date) &&
           ((date[-1] == '+' || date[-1] == '-'))) {
          /* four digits and a value less than or equal to 1400 (to take into
             account all sorts of funny time zone diffs) and it is preceded
             with a plus or minus. This is a time zone indication.  1400 is
             picked since +1300 is frequently used and +1400 is mentioned as
             an edge number in the document "ISO C 200X Proposal: Timezone
             Functions" at http://david.tribble.com/text/c0xtimezone.html If
             anyone has a more authoritative source for the exact maximum time
             zone offsets, please speak up! */
          found = true;
          tzoff = (val/100 * 60 + val%100)*60;

          /* the + and - prefix indicates the local time compared to GMT,
             this we need ther reversed math to get what we want */
          tzoff = date[-1]=='+'?-tzoff:tzoff;
        }

        if(((end - date) == 8) &&
           (yearnum == -1) &&
           (monnum == -1) &&
           (mdaynum == -1)) {
          /* 8 digits, no year, month or day yet. This is YYYYMMDD */
          found = true;
          yearnum = val/10000;
          monnum = (val%10000)/100-1; /* month is 0 - 11 */
          mdaynum = val%100;
        }

        if(!found && (dignext == DATE_MDAY) && (mdaynum == -1)) {
          if((val > 0) && (val<32)) {
            mdaynum = val;
            found = true;
          }
          dignext = DATE_YEAR;
        }

        if(!found && (dignext == DATE_YEAR) && (yearnum == -1)) {
          yearnum = val;
          found = true;
          if(yearnum < 1900) {
            if(yearnum > 70)
              yearnum += 1900;
            else
              yearnum += 2000;
          }
          if(mdaynum == -1)
            dignext = DATE_MDAY;
        }

        if(!found)
          return PARSEDATE_FAIL;

        date = end;
      }
    }

    part++;
  }

  if(-1 == secnum)
    secnum = minnum = hournum = 0; /* no time, make it zero */

  if((-1 == mdaynum) ||
     (-1 == monnum) ||
     (-1 == yearnum))
    /* lacks vital info, fail */
    return PARSEDATE_FAIL;

#if SIZEOF_TIME_T < 5
  /* 32 bit time_t can only hold dates to the beginning of 2038 */
  if(yearnum > 2037) {
    *output = 0x7fffffff;
    return PARSEDATE_LATER;
  }
#endif

  if(yearnum < 1970) {
    *output = 0;
    return PARSEDATE_SOONER;
  }

  tm.tm_sec = secnum;
  tm.tm_min = minnum;
  tm.tm_hour = hournum;
  tm.tm_mday = mdaynum;
  tm.tm_mon = monnum;
  tm.tm_year = yearnum - 1900;

  /* my_timegm() returns a time_t. time_t is often 32 bits, even on many
     architectures that feature 64 bit 'long'.

     Some systems have 64 bit time_t and deal with years beyond 2038. However,
     even on some of the systems with 64 bit time_t mktime() returns -1 for
     dates beyond 03:14:07 UTC, January 19, 2038. (Such as AIX 5100-06)
  */
  t = my_timegm(&tm);

  /* time zone adjust (cast t to int to compare to negative one) */
  if(-1 != (int)t) {

    /* Add the time zone diff between local time zone and GMT. */
    long delta = (long)(tzoff!=-1?tzoff:0);

    if((delta>0) && (t + delta < t))
      return -1; /* time_t overflow */

    t += delta;
  }

  *output = t;

  return PARSEDATE_OK;
}


std::string updater_impl::make_http_last_modified(const std::string& file)
{
	std::string str;
	fs::path p(file);
	if (fs::exists(p))
	{
		std::time_t t = fs::last_write_time(p);
		tm* gmt = gmtime((const time_t*)&t);
		char time_buf[512] = { 0 };
		strftime(time_buf, 200, "%a, %d %b %Y %H:%M:%S GMT\r\n", gmt);
		str = time_buf;
		str = "If-Modified-Since: " + str;
	}

	return str;
}

bool updater_impl::parser_http_last_modified(const std::string& str, struct tm* time)
{
   std::string date;
   time_t t;

   std::size_t pos = str.find(':');
   if (pos != std::string::npos)
      date = str.substr(pos + 1, str.length() - pos);
   else
      date = str;
   if (parsedate(date.c_str(), &t) != PARSEDATE_OK)
      return false;
   struct tm* tm = localtime(&t);
   if (!tm)
      return false;
   *time = *tm;

   return true;
}
