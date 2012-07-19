#include <direct.h>
#include <errno.h>

#include "uncompress.h"

#ifndef GZ_SUFFIX
#define GZ_SUFFIX ".gz"
#endif
#define SUFFIX_LEN (sizeof(GZ_SUFFIX)-1)

#define BUFLEN      16384
#define MAX_NAME_LEN 4096
#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef WIN32
#include <windows.h>
void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date)
{
   HANDLE hFile;
   FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;
   hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
      0, NULL, OPEN_EXISTING, 0, NULL);
   GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);
   DosDateTimeToFileTime((WORD)(dosdate >> 16),(WORD)dosdate, &ftLocal);
   LocalFileTimeToFileTime(&ftLocal, &ftm);
   SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);
   CloseHandle(hFile);
}
#endif // WIN32

int mymkdir(const char* dirname)
{
   int ret = 0;
   ret = mkdir(dirname);
   return ret;
}

int makedir(char *newdir)
{
   char *buffer ;
   char *p;
   int  len = (int)strlen(newdir);

   if (len <= 0)
      return 0;

   buffer = (char*)malloc(len + 1);
   strcpy(buffer, newdir);

   if (buffer[len - 1] == '/')
      buffer[len - 1] = '\0';

   if (mymkdir(buffer) == 0)
   {
      free(buffer);
      return 1;
   }

   p = buffer+1;
   while (1)
   {
      char hold;

      while(*p && *p != '\\' && *p != '/')
         p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
      {
         printf("couldn't create directory %s\n", buffer);
         free(buffer);
         return 0;
      }
      if (hold == 0)
         break;
      *p++ = hold;
   }
   free(buffer);
   return 1;
}

int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, 
   int* popt_overwrite, const char* extract_path, const char* password, int opt_close/* = 1*/)
{
   char filename_inzip[256];
   char path[256];
   char* filename_withoutpath;
   char* p;
   int err = UNZ_OK;
   FILE *fout = NULL;
   void* buf;
   uInt size_buf;

   unz_file_info file_info;
   uLong ratio = 0;
   err = unzGetCurrentFileInfo(uf, &file_info, 
      filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

   if (err != UNZ_OK) {
      printf("error %d with zipfile in unzGetCurrentFileInfo\n", err);
      return err;
   }

   size_buf = WRITEBUFFERSIZE;
   buf = (void*)malloc(size_buf);
   if (buf == NULL) {
      printf("Error allocating memory\n");
      return UNZ_INTERNALERROR;
   }

   p = filename_withoutpath = filename_inzip;
   while ((*p) != '\0') {
      if (((*p)=='/') || ((*p)=='\\'))
         filename_withoutpath = p + 1;
      p++;
   }

   if ((*filename_withoutpath) == '\0') {
      if ((*popt_extract_without_path) == 0) {
         printf("creating directory: %s\n", filename_inzip);
         mymkdir(filename_inzip);
      }
   } else {
      const char* write_filename;
      int skip = 0;

      if (extract_path != NULL)
         strncpy(path, extract_path, 256);

      if ((*popt_extract_without_path) == 0) {
         if (extract_path != NULL) {
            strcat(path, filename_inzip);
            write_filename = path;
         } else {
            write_filename = filename_inzip;
         }			
      } else {
         if (extract_path != NULL) {
            strcat(path, filename_withoutpath);
            write_filename = path;
         } else {
            write_filename = filename_withoutpath;
         }
      }

      err = unzOpenCurrentFilePassword(uf, password);
      if (err != UNZ_OK)
         printf("error %d with zipfile in unzOpenCurrentFilePassword\n", err);

      // 忽略参数,始终为覆盖.
      *popt_overwrite = 1;

      if (((*popt_overwrite) == 0) && (err == UNZ_OK)) {
         char rep = 0;
         FILE* ftestexist;
         ftestexist = fopen(write_filename, "rb");
         if (ftestexist != NULL) {
            fclose(ftestexist);
            do {
               char answer[128];
               int ret;

               printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ", write_filename);
               ret = scanf("%1s", answer);
               if (ret != 1)
                  exit(EXIT_FAILURE);
               rep = answer[0] ;
               if ((rep >= 'a') && (rep <= 'z'))
                  rep -= 0x20;
            }
            while ((rep != 'Y') && (rep != 'N') && (rep != 'A'));
         }

         if (rep == 'N')
            skip = 1;

         if (rep == 'A')
            *popt_overwrite = 1;
      }

      if ((skip == 0) && (err == UNZ_OK))
      {
         fout = fopen(write_filename, "wb");

         /* some zipfile don't contain directory alone before file */
         if ((fout == NULL) && ((*popt_extract_without_path) == 0) &&
            (filename_withoutpath != (char*)filename_inzip)) {
            char c = *(filename_withoutpath - 1);
            *(filename_withoutpath - 1) = '\0';
            makedir((char*)write_filename);
            *(filename_withoutpath - 1) = c;
            fout = fopen(write_filename, "wb");
         }

         if (fout == NULL)
            printf("error opening %s\n", write_filename);
      }

      if (fout != NULL) {
         printf("extracting: %s\n", write_filename);

         do {
            err = unzReadCurrentFile(uf, buf, size_buf);
            if (err < 0) {
               printf("error %d with zipfile in unzReadCurrentFile\n", err);
               break;
            }
            if (err > 0)
               if (fwrite(buf, err, 1, fout) != 1) {
                  printf("error in writing extracted file\n");
                  err = UNZ_ERRNO;
                  break;
               }
         }
         while (err > 0);
         if (fout)
            fclose(fout);

         if (err == 0)
            change_file_date(write_filename, file_info.dosDate,
            file_info.tmu_date);
      }

      if (opt_close == 1) {
         if (err == UNZ_OK) {
            err = unzCloseCurrentFile(uf);
            if (err != UNZ_OK)
               printf("error %d with zipfile in unzCloseCurrentFile\n", err);
         }
         else
            unzCloseCurrentFile(uf); /* don't lose the error */
      }
   }

   free(buf);

   return err;
}

int do_extract(unzFile uf, int opt_extract_without_path, 
   int opt_overwrite, const char* extract_path, const char* password)
{
   uLong i;
   unz_global_info gi;
   int err;
   FILE* fout = NULL;

   err = unzGetGlobalInfo(uf, &gi);
   if (err != UNZ_OK)
      printf("error %d with zipfile in unzGetGlobalInfo \n", err);

   for (i = 0; i < gi.number_entry; i++) {
      err = do_extract_currentfile(uf, &opt_extract_without_path, 
         &opt_overwrite, extract_path, password);
      if (err != UNZ_OK)
         break;

      if ((i + 1) < gi.number_entry) {
         err = unzGoToNextFile(uf);
         if (err != UNZ_OK) {
            printf("error %d with zipfile in unzGoToNextFile\n", err);
            break;
         }
      }
   }

   return err;
}

int do_extract_onefile(unzFile uf, const char* filename, int opt_extract_without_path, 
   int opt_overwrite, const char* extract_path, const char* password)
{
   int err = UNZ_OK;
   if (unzLocateFile(uf,filename,CASESENSITIVITY) != UNZ_OK)
   {
      printf("file %s not found in the zipfile\n",filename);
      return 2;
   }

   if (do_extract_currentfile(uf,&opt_extract_without_path,
      &opt_overwrite,
      extract_path,
      password) == UNZ_OK)
      return 0;
   else
      return 1;
}

int do_extract_gz(const char* file)
{
   char *infile, *outfile;
   char name_buf[MAX_NAME_LEN];
   FILE  *out;
   gzFile in;
   size_t name_len = strlen(file);

   if (name_len + strlen(GZ_SUFFIX) >= sizeof(name_buf))
      return -1;

   strcpy(name_buf, file);

   if (name_len > SUFFIX_LEN && strcmp(file + name_len - SUFFIX_LEN, GZ_SUFFIX) == 0) {
      infile = (char *)file;
      outfile = name_buf;
      outfile[name_len - 3] = '\0';
   } else {
      outfile = (char *)file;
      infile = name_buf;
      strcat(infile, GZ_SUFFIX);
   }

   in = gzopen(infile, "rb");
   if (in == NULL)
      return -1;

   out = fopen(outfile, "wb");
   if (out == NULL)
      return -1;

   char buf[BUFLEN];
   int len;

   for (;;) {
      len = gzread(in, buf, sizeof(buf));
      if (len < 0) 
         return 0;
      if (len == 0)
         break;

      if ((int)fwrite(buf, 1, (unsigned)len, out) != len)
         return -1;
   }
   if (fclose(out)) 
      return -1;
   if (gzclose(in) != Z_OK) 
      return -1;
   return 0;
}

int do_extract_zip(const char* file, const char* extract_path/* = NULL*/)
{
   unzFile uf = NULL;
   const char *zipfilename = file;
   uf = unzOpen(zipfilename);
   if (!uf)
      return -1;
   do_extract(uf, 0, 1, extract_path, NULL);
   if (unzClose(uf) != UNZ_OK)
      return -1;
   return 0;
}
