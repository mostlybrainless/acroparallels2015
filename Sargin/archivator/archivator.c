//
//  archivator.c
//  archivator
//
//  Created by Daniil Sargin on 08/04/15.
//  Copyright (c) 2015 Даниил Саргин. All rights reserved.
//

#include "../include/archivator.h"
#include <zlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define CHUNK 16384


typedef struct file_info_s {
    uLong length;
    uLong length_compr;
    int is_last;
} file_info;

int deflate_chunk(uLong chunk_length, char * src_chunk, int dest_fd, int is_last)
{
    file_info file_inf;
    char * deflate_buff;
    file_inf.length = chunk_length;
    file_inf.length_compr = compressBound(chunk_length);
    file_inf.is_last = is_last;
    deflate_buff = (char *)malloc(file_inf.length_compr * sizeof(char));
    if (deflate_buff == NULL) {
        nlog(LOG_ERROR, &LOGC, "deflate: malloc for buffer failed");
        return -1;
    }
    if (compress((Bytef *)deflate_buff, &file_inf.length_compr, (Bytef *)src_chunk, file_inf.length) != Z_OK) {
        nlog(LOG_ERROR, &LOGC, "deflate: compession failed");
        return -1;
    }
    
    if (write(dest_fd, &file_inf, sizeof(file_inf)) == -1) {
        nlog(LOG_ERROR, &LOGC, "deflate: head of block write failed");
        return -1;
    }
    if (write(dest_fd, deflate_buff, file_inf.length_compr) == -1) {
        nlog(LOG_ERROR, &LOGC, "deflate: block write failed");
        return -1;
    }
    
    free(deflate_buff);
    
    return 1;
}

int arch_deflate(const char * src_name)
{
    struct stat src_stat;
    int src_fd, dest_fd;
    char * dest_name;
    char * src_data;
    
    src_fd = open(src_name, O_RDONLY);
    if (src_fd == -1) {
        nlog(LOG_ERROR, &LOGC, "arch_deflate: open src file failed");
        return -1;
    }
    
    if (fstat(src_fd, &src_stat) == -1) {
        nlog(LOG_ERROR, &LOGC, "arch_deflate: stat mining of src file failed");
        return -1;
    }
    
    ssize_t src_length = src_stat.st_size;
    
    src_data = mmap(NULL, src_length, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);
    if (src_data == MAP_FAILED) {
        nlog(LOG_ERROR, &LOGC,"arch_deflate: mmap src data failed");
        return -1;
    }
    
    dest_name = (char *) malloc((strlen(src_name) + 9) * sizeof(char));
    if (dest_name == NULL) {
        nlog(LOG_ERROR, &LOGC, "arch_deflate: malloc for dest name failed");
        return -1;
    }
    strcat(dest_name, src_name);
    strcat(dest_name, ".archive");
    
    dest_fd = open(dest_name, O_WRONLY | O_CREAT, 0666);
    if (dest_fd == -1) {
        nlog(LOG_ERROR, &LOGC, "arch_deflate: open dest file failed");
        return -1;
    }
    free(dest_name);
    
    while (src_length != 0) {
        if (src_length > CHUNK) {
            deflate_chunk(CHUNK, src_data, dest_fd, 0);
            src_data += CHUNK;
            src_length -= CHUNK;
        } else {
            deflate_chunk(src_length, src_data, dest_fd, 1);
            src_length = 0;
        }
    }
    munmap(src_data, src_stat.st_size);
    
    close(dest_fd);
    
    return 1;
}

uLong inflate_chunk(char * src_chunk, int dest_fd)
{
    file_info * file_inf = (file_info *)malloc(sizeof(file_info));
    if (file_inf == NULL) {
        nlog(LOG_ERROR, &LOGC, "inflate: malloc for file_inf failed");
        return -1;
    }
    char * inflate_buff;
    memcpy(file_inf, src_chunk, sizeof(file_info));
    inflate_buff = (char *)malloc(CHUNK * sizeof(char));
    if (inflate_buff == NULL) {
        nlog(LOG_ERROR, &LOGC, "inflate: malloc for buffer failed");
        return -1;
    }
    uLong offset = sizeof(file_info);
    if (uncompress((Bytef *)inflate_buff, &file_inf->length, (Bytef *)(src_chunk + offset), file_inf->length_compr) != Z_OK) {
        nlog(LOG_ERROR, &LOGC, "inflate: uncompression failed");
        return -1;
    }
    if (write(dest_fd, inflate_buff, file_inf->length) == -1) {
        nlog(LOG_ERROR, &LOGC, "inflate: write of block failed");
        return -1;
    }
    free(inflate_buff);
    if (file_inf->is_last == 1) {
        free(file_inf);
        return 0;
    } else {
        offset += file_inf->length_compr;
        free(file_inf);
        return offset;
    }
}

int arch_inflate(const char * src_name)
{
    struct stat src_stat;
    int src_fd, dest_fd;
    char * src_data;
    char * dest_name;
    
    src_fd = open(src_name, O_RDONLY);
    if (src_fd == -1) {
        nlog(LOG_ERROR, &LOGC, "arch_inflate: open src file failed");
        return -1;
    }
    if (fstat(src_fd, &src_stat) == -1) {
        nlog(LOG_ERROR, &LOGC, "arch_inflate: stat mining of src file failed");
        return -1;
    }
    
    src_data = mmap(NULL, src_stat.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);
    if (src_data == MAP_FAILED) {
        nlog(LOG_ERROR, &LOGC,"arch_inflate: mmap src data failed");
        return -1;
    }
    
    size_t len = strlen(src_name);
    dest_name = (char *)malloc((len - 7) * sizeof(char));
    if (dest_name == NULL) {
        nlog(LOG_ERROR, &LOGC, "arch_inflate: malloc for dest name failed");
        return -1;
    }
    memcpy(dest_name, src_name, len - 8);
    dest_name[len - 8] = 0;
    
    dest_fd = open(dest_name, O_WRONLY | O_CREAT, 0666);
    if (dest_fd == -1) {
        nlog(LOG_ERROR, &LOGC, "arch_inflate: open dest file failed");
        return -1;
    }
    uLong offs = 0;
    do {
        offs = inflate_chunk(src_data, dest_fd);
        src_data += offs;
    } while (offs != 0);
    
    munmap(src_data, src_stat.st_size);
    close(dest_fd);
    
    return 1;
}