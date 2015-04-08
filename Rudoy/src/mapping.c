#include "defines.h"

#define CHUNK 16384




void*  mapFileToMemory( int fd, struct stat *info )
{
	void *src;
	if ( fstat( fd, info) == -1 )
		RETURN_ERROR("fstat error", NULL);
	if (( src = mmap(0, info->st_size, PROT_READ, MAP_PRIVATE, fd, 0) ) == MAP_FAILED)
		RETURN_ERROR("mapFileToMemory: mmap error", NULL);
	return src;
}


int mycompress(char *source, char *dest, int level , file_block *info)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    char *tmp;
    tmp = dest;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;
    int i = 0, last_i;
    last_i = info->size_before / CHUNK;                                            // count iterations
    info->size_after = 0;
    do {
        
        if ( i == last_i)
        {
            flush = Z_FINISH;
            strm.avail_in =  info->size_before % CHUNK;                           // in last iteration count of bytes
        }
        else
        {
            flush = Z_NO_FLUSH;
            strm.avail_in = CHUNK;
        }
        strm.next_in = &source[i*CHUNK];
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);
            have = CHUNK - strm.avail_out; 
            memcpy(tmp, out, have);
            tmp += have;
            info->size_after += have;
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);
        i++;
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);
    (void)deflateEnd(&strm);
    return Z_OK;
}


int mydecompress(char *source, char *dest , file_block *info)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];
    char *tmp;
    tmp = dest;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
    int i = 0, last_i;
    last_i = info->size_after / CHUNK;
    size_t size = 0;
    do {
        if ( i == last_i )
        {
            strm.avail_in =  info->size_after % CHUNK;
        }
        else if ( i == last_i + 1 )
        {
            break;
        }
        else 
            strm.avail_in = CHUNK;
        strm.next_in = &source[i*CHUNK];
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            memcpy(tmp, out, have);
            tmp += have;
            size += have;
        } while (strm.avail_out == 0);
        i++;
    } while (ret != Z_STREAM_END);
    if ( size != info->size_before)
        ALOGF("mydecompress: file \"%s\" is incorrect decompressed, size before = %d, size now = %d", info->path, info->size_before, size );
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END  ? Z_OK : Z_DATA_ERROR;
}
