#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"

       #include <sys/mman.h>
       #include <sys/stat.h>
       #include <fcntl.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <unistd.h>
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

       
       
       #define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)


#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}


size_t deflate_mmap(char *source, size_t sourceLen, char *dest, size_t destLen, int level){
    int ret, flush;
    size_t have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    size_t numberOfPreprocessedChanks = 0;
    size_t numberOfCompressedBytes = 0;
    
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        
        size_t bytesLeft = sourceLen - numberOfPreprocessedChanks*CHUNK ;
        strm.avail_in = (bytesLeft>CHUNK) ? CHUNK : bytesLeft; // fread(in, 1, CHUNK, source);
        //if (ferror(source)) {(void)deflateEnd(&strm);return Z_ERRNO;}
        //flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        flush = (bytesLeft < CHUNK ) ? Z_FINISH : Z_NO_FLUSH;
        
        strm.next_in = source + numberOfPreprocessedChanks*CHUNK;
        numberOfPreprocessedChanks++;
        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        
        do {//sleep(1);
            size_t current_out_buf_size = (destLen - numberOfCompressedBytes) > CHUNK ? CHUNK : (destLen - numberOfCompressedBytes) ;
            strm.avail_out = current_out_buf_size ;
            printf("bytesLeft = %d\n", (int) strm.avail_out);
            strm.next_out = dest+numberOfCompressedBytes;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = current_out_buf_size -(size_t) strm.avail_out;
            printf("Have = %d\n", (int) have);
            numberOfCompressedBytes +=have;
            //if (fwrite(out, 1, have, dest) 
                //!= have || ferror(dest)) { (void)deflateEnd(&strm);return Z_ERRNO; }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return numberOfCompressedBytes;
}


/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

/* compress or decompress from stdin to stdout */
int main(int argc, char **argv)
{
    int ret;

    /* avoid end-of-line conversions */

    /* do compression if no arguments */
    if (argc == 3) {
           char *addr;
           int fdr, fdw;
           struct stat sb;
           size_t out_len =6000*1000;
           out_len*=1000 ;
           char *outbuff= (char*)malloc(out_len );
           off_t offset, pa_offset;
           size_t length;
           fdr = open( argv[1], O_RDONLY);
           fstat(fdr, &sb);
           length = sb.st_size;
           addr = mmap(NULL, length, PROT_READ,
                       MAP_PRIVATE, fdr, 0);
           printf("input length = %lld\n",  length);
           length = deflate_mmap(addr, length , outbuff, out_len, 7 );
           printf("output length = %lld\n",  length);
           size_t len;
           fdw = open( argv[2], O_CREAT | O_WRONLY, 0666);
           
           while ( len = write(fdw, outbuff, length)){
                length -=len;
                outbuff +=len;
            }
           //ret = def(stdin, stdout, Z_DEFAULT_COMPRESSION);
           if (ret != Z_OK)
               zerr(ret);
           return ret;
    }

    /* do decompression if -d specified */
    else if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        ret = inf(stdin, stdout);
        if (ret != Z_OK)
            zerr(ret);
        return ret;
    }

    /* otherwise, report usage */
    else {
        fputs("zpipe usage: zpipe [-d] source dest\n", stderr);
        return 1;
    }
}
