#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "zlib.h"
#include "../log/log.h"
#include "arch.h"

const int BLOCK_SIZE = 1 << 10;
log_info_t LOG;

int compress_block(int fd, char * src, int length_src)
{
    int length_dest, ret;
    char * buf;
    block_info_t * head = (block_info_t*)calloc(1, sizeof(block_info_t));
    if (head == NULL) { log(&LOG, ERROR, "Allocate memory for head of block"); return -1; }

    length_dest = compressBound(length_src);
    buf = (char*)malloc(length_dest * sizeof(char));
    if (buf == NULL) { log(&LOG, ERROR, "Allocate memory for compressed block"); return -1; }
   
    ret = compress(buf, (uLongf*)&length_dest, src, length_src);
    if (ret != Z_OK) { log(&LOG, ERROR, "Compression failed"); return -1; }
    head->compr_size = length_dest;

    ret = write(fd, head, sizeof(block_info_t));
    if (ret == -1) { log(&LOG, ERROR, "Write head of block"); return -1; }

    ret = write(fd, buf, length_dest);
    if (ret == -1) { log(&LOG, ERROR, "Write compressed block"); return -1; }

    log(&LOG, DEBUG, "src = %d, dst = %d, wr = %d", length_src, length_dest, ret);
    log(&LOG, INFO, "Block successfully compressed and written");

    free(buf);
    free(head);
    return 1;
}

int compress_file(const char * file_from, const char * file_to)
{
    int fd_src, fd_dest, length_src, ret;
    struct stat info;
    char * map_begin, * map_curr;
    file_info_t * head = (file_info_t*)calloc(1, sizeof(file_info_t));
    if (head == NULL) { log(&LOG, ERROR, "Allocate memory for head of file"); return -1; }

    fd_src = open(file_from, O_RDONLY);
    if (fd_src == -1) { log(&LOG, ERROR, "Open source file"); return -1; }
    ret = fstat(fd_src, &info);
    if (ret == -1) { log(&LOG, ERROR, "Get info from source file"); return -1; }
    length_src = info.st_size;

    map_begin = mmap(NULL, length_src, PROT_READ, MAP_PRIVATE, fd_src, 0);
    map_curr = map_begin;
    if (map_begin == MAP_FAILED) { log(&LOG, ERROR, "Map source file"); return -1; }

    fd_dest = open(file_to, O_WRONLY | O_CREAT | O_TRUNC, info.st_mode);
    if (fd_dest == -1) { log(&LOG, ERROR, "Open/Creating destination file"); return -1; }

    head->mode = info.st_mode;
    head->block_num = 0;
    ret = lseek(fd_dest, sizeof(file_info_t), SEEK_SET);
    if (ret == -1) { log(&LOG, ERROR, "Set offset to retain space for head"); return -1; }

    while (length_src > 0)
    {
        ret = compress_block(fd_dest, map_curr, min(BLOCK_SIZE, length_src));
        if (ret == -1) { log(&LOG, ERROR, "Compression of block failed"); return -1; }
        map_curr += BLOCK_SIZE;
        length_src -= BLOCK_SIZE;
        head->block_num++;
    }

    ret = lseek(fd_dest, 0, SEEK_SET);
    if (ret == -1) { log(&LOG, ERROR, "Set offset = 0 to write head"); return -1; }
    ret = write(fd_dest, head, sizeof(file_info_t));
    if (ret == -1) { log(&LOG, ERROR, "Write head of file"); return -1; }
    
    log(&LOG, INFO, "File successfully compressed and written");

    munmap(map_begin, info.st_size);
    free(head);
    close(fd_src);
    close(fd_dest);
    return 0;
}

int decompress_block(int fd, char * src)
{
    int length_src, length_dest, offset, ret;
    char * buf;
    block_info_t * head = (block_info_t*)calloc(1, sizeof(block_info_t));
    if (head == NULL) { log(&LOG, ERROR, "Allocate memory for head of block"); return -1; }

    offset = sizeof(block_info_t);
    memcpy(head, src, offset);
    src += offset;
    length_dest = BLOCK_SIZE;
    length_src = head->compr_size;
   
    buf = (char*)malloc(length_dest * sizeof(char));
    if (buf == NULL) { log(&LOG, ERROR, "Allocate memory for uncompressed block"); return -1; }
    
    ret = uncompress(buf, (uLongf*)&length_dest, src, length_src);
    if (ret != Z_OK) { log(&LOG, ERROR, "Deompression failed"); return -1; }
    
    ret = write(fd, buf, length_dest);
    if (ret == -1) { log(&LOG, ERROR, "Write in file uncompressed block"); return -1; }
    
    log(&LOG, DEBUG, "src = %d, dst = %d, wr = %d", length_src, length_dest, ret);
    log(&LOG, INFO, "Block successfully decompressed and written");
    
    free(buf);
    free(head);
    return length_src + offset;
}

int decompress_file(const char * file_from, const char * file_to)
{
    int fd_src, fd_dest, length_src, ret, offset, next_block;
    struct stat info;
    char * map_begin, * map_curr;
    file_info_t * head = (file_info_t*)calloc(1, sizeof(file_info_t));
    if (head == NULL) { log(&LOG, ERROR, "Allocate memory for head of file"); return -1; }

    fd_src = open(file_from, O_RDONLY);
    if (fd_src == -1) { log(&LOG, ERROR, "Open source file"); return -1; }
    ret = fstat(fd_src, &info);
    if (ret == -1) { log(&LOG, ERROR, "Get info from source file"); return -1; }
    length_src = info.st_size;

    map_begin = mmap(NULL, length_src, PROT_READ, MAP_PRIVATE, fd_src, 0);
    map_curr = map_begin;
    if (map_begin == MAP_FAILED) { log(&LOG, ERROR, "Map source file"); return -1; }

    offset = sizeof(file_info_t);
    memcpy(head, map_curr, offset);
    map_curr += offset;
    length_src -= offset;

    fd_dest = open(file_to, O_WRONLY | O_CREAT | O_TRUNC, head->mode);
    if (fd_dest == -1) { log(&LOG, ERROR, "Open/Creating destination file"); return -1; }

    while (head->block_num-- > 0)
    {
        next_block = decompress_block(fd_dest, map_curr);
        if (next_block == -1) { log(&LOG, ERROR, "Decompression of block failed"); return -1; }
        map_curr += next_block;
    }
    
    log(&LOG, INFO, "File successfully decompressed and written");

    munmap(map_begin, info.st_size);
    free(head);
    close(fd_src);
    close(fd_dest);
    return 0;
}

int is_have_postfix(const char * name)
{
    int len = strlen(name);
    if (len > 5 && strcmp(name + len - 5, ".arch") == 0)
        return 1;
    else
        return 0;
}

int main_arch(int argc, const char * argv[])
{
    char * file_to;
    log_info_t * log_info = log_init("log.txt", DEBUG);
    LOG = *log_info;
    int done = 0;

    if (argc < 3 || argc > 5)
        printf("Wrong number of parameters.\n");
    else if (argc == 3 && strcmp(argv[1], "-c") == 0)
    {
        file_to = (char*)malloc((strlen(argv[2]) + 7) * sizeof(char));
        strcpy(file_to, argv[2]);
        strcat(file_to, ".arch");
        compress_file(argv[2], file_to);
        free(file_to);
        done = 1;
    }
    else if (argc == 4 && strcmp(argv[1], "-c") == 0)
    {
        if (is_have_postfix(argv[3]))
        {
            compress_file(argv[2], argv[3]);
            done = 1;
        }
        else
            printf("Have to compress into *.arch.\n");
    }
    else if (argc == 3 && strcmp(argv[1], "-u") == 0)
    {
        if (is_have_postfix(argv[2]))
        {
            file_to = (char*)malloc(strlen(argv[2]) * sizeof(char));
            strncpy(file_to, argv[2], strlen(argv[2]) - 5);
            strcat(file_to, "\0");
            decompress_file(argv[2], file_to);
            free(file_to);
            done = 1;
        }
    }
    else if (argc == 4 && strcmp(argv[1], "-u") == 0)
    {
        if (is_have_postfix(argv[2]))
        {
            decompress_file(argv[2], argv[3]);
            done = 1;
        }
        else
            printf("Have to decompress from *.arch.\n");
    }
    else
        printf("Wrong parameters.\n");
    
    log_exit(log_info);
    return done;
}
