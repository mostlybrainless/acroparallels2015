//
//  main.c
//  archivator
//
//  Created by Daniil Sargin on 31/03/15.
//  Copyright (c) 2015 Даниил Саргин. All rights reserved.
//


#include <stdio.h>
#include <string.h>
#include "include/archivator.h"
#include "include/logger.h"

int main(int argc, const char * argv[])
{
    log_context logc = initlog(LOG_DEBUG, "log.txt", "w");
    LOGC = logc;
    if (argc < 3 || argc > 3) {
        perror("no output file name or input file name \n");
        return -1;
    }
    if (strcmp(argv[1], "compress") == 0) {
        arch_deflate(argv[2]);
    } else if (strcmp(argv[1], "decompress") == 0) {
        arch_inflate(argv[2]);
    }
    deinitlog(&logc);
    return 0;
}
