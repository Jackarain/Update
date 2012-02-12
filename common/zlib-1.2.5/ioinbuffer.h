/* iowin32.h -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API
 
   Copyright (C) 1998-2009 jack.wgm
*/




#ifdef __cplusplus
extern "C" {
#endif

void fill_buffer_func OF((zlib_filefunc_def* pzlib_filefunc_def));
void update_file_length OF((uLong filelength, voidpf stream));

#ifdef __cplusplus
}
#endif
