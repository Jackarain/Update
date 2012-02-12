/* ioinbuffer.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API

   Copyright (C) 1998-2009 jack.wgm
*/

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "zlib.h"
#include "ioapi.h"
#include "ioinbuffer.h"



typedef struct
{
    char* fhead;
    char* curpos;
	uLong filesize;
} MEMFILEIO;

void update_file_length(filelength, stream)
				  uLong filelength;
				  voidpf stream;
{
	MEMFILEIO* ptr = (MEMFILEIO*)stream;
	if (ptr)
	{
		ptr->filesize = filelength;
	}
}

// 打开文件,返回MEMFILEIO指针.
voidpf ZCALLBACK buffer_open_file_func OF((
   voidpf opaque,
   const char* filebuffer,
   int mode));

// 读取文件.
uLong ZCALLBACK buffer_read_file_func OF((
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size));
   
// 写文件.   
uLong ZCALLBACK buffer_write_file_func OF((
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size));

// 得到文件当前指针.
long ZCALLBACK buffer_tell_file_func OF((
   voidpf opaque,
   voidpf stream));

// 移动文件指针.
long ZCALLBACK buffer_seek_file_func OF((
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin));

// 关闭文件.
int ZCALLBACK buffer_close_file_func OF((
   voidpf opaque,
   voidpf stream));
   
// 操作失败处理函数.
int ZCALLBACK buffer_error_file_func OF((
   voidpf opaque,
   voidpf stream));
   
   
voidpf ZCALLBACK buffer_open_file_func (opaque, filebuffer, mode)
   voidpf opaque;
   const char* filebuffer;
   int mode;
{
	MEMFILEIO* ptr = NULL;
	
	if (filebuffer!=NULL)
	{		
		ptr = malloc(sizeof(MEMFILEIO));
		
		if (ptr==NULL)
			return NULL;
		ptr->fhead = (char*)filebuffer;
		ptr->curpos = ptr->fhead;
		ptr->filesize = (uLong)0;
	}
	
    return ptr;
}
   
uLong ZCALLBACK buffer_read_file_func (opaque, stream, buf, size)
   voidpf opaque;
   voidpf stream;
   void* buf;
   uLong size;
{
	MEMFILEIO* ptr = (MEMFILEIO*)stream;
	if ((stream!=NULL) && (buf!=NULL))
	{
		memcpy(buf, ptr->curpos, size);
		ptr->curpos += size;		
		return size;
	}
	
	return 0;
}

uLong ZCALLBACK buffer_write_file_func (opaque, stream, buf, size)
   voidpf opaque;
   voidpf stream;
   const void* buf;
   uLong size;
{
	MEMFILEIO* ptr = (MEMFILEIO*)stream;
	if ((stream!=NULL) && (buf!=NULL))
	{
		memcpy(ptr->curpos, buf, size);
		ptr->curpos += size;
		
		// 更新文件长度.
		if ((uLong)(ptr->curpos-ptr->fhead) > ptr->filesize)
		{
			ptr->filesize = (ptr->curpos-ptr->fhead);
		}
		return size;
	}
	
	return 0;
}

long ZCALLBACK buffer_tell_file_func (opaque, stream)
   voidpf opaque;
   voidpf stream;
{
    uLong ret=0;
	MEMFILEIO* ptr = (MEMFILEIO*)stream;
	
    if (stream!=NULL)
	{
		ret = (ptr->curpos - ptr->fhead);
		return ret;
	}
	
	return 0;
}

long ZCALLBACK buffer_seek_file_func (opaque, stream, offset, origin)
   voidpf opaque;
   voidpf stream;
   uLong offset;
   int origin;
{
	MEMFILEIO* ptr = (MEMFILEIO*)stream;
	unsigned long dwMoveMethod=0xFFFFFFFF;
	
	if (stream==NULL)
		return -1;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        {
			if (((uLong)((ptr->curpos+offset)-ptr->fhead) > ptr->filesize) ||
				((uLong)((ptr->curpos+offset)-ptr->fhead) < 0))
			{
				return -1;
			}
			
			ptr->curpos += offset;
		}
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        {
			if (((ptr->fhead+ptr->filesize)-offset) < ptr->fhead)
			{
				return -1;
			}
			
			ptr->curpos = ((ptr->fhead+ptr->filesize)-offset);
		}
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        {
			if ((ptr->fhead+offset) > (ptr->fhead+ptr->filesize))
			{
				return -1;
			}
			ptr->curpos = (ptr->fhead+offset);
		}
        break;
    default: return -1;
    }
	
	return 0;
}

int ZCALLBACK buffer_close_file_func (opaque, stream)
   voidpf opaque;
   voidpf stream;
{
    int ret=-1;
	
    if (stream!=NULL)
    {
        free(stream);
		return 0;
    }
    return ret;
}

int ZCALLBACK buffer_error_file_func (opaque, stream)
   voidpf opaque;
   voidpf stream;
{
    return -1;
}

void fill_buffer_func (pzlib_filefunc_def)
  zlib_filefunc_def* pzlib_filefunc_def;
{
	pzlib_filefunc_def->zopen_file = buffer_open_file_func;	
    pzlib_filefunc_def->zread_file = buffer_read_file_func;
    pzlib_filefunc_def->zwrite_file = buffer_write_file_func;
    pzlib_filefunc_def->ztell_file = buffer_tell_file_func;
    pzlib_filefunc_def->zseek_file = buffer_seek_file_func;
    pzlib_filefunc_def->zclose_file = buffer_close_file_func;
    pzlib_filefunc_def->zerror_file = buffer_error_file_func;
    pzlib_filefunc_def->opaque=NULL;
}


