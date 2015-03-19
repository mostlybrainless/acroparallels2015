//
//  log.c
//  log
//
//  Created by Daniil Sargin on 17/03/15.
//  Copyright (c) 2015 Даниил Саргин. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include "logger.h"


int main(int argc, const char * argv[]) {
    struct log_context logc = initlog(3, "/Users/Avterks/Desktop/log.txt", "w");

    nlog(3, &logc, "logged %d and %.1f and %d", 5, 0.5, 5);
    sleep(1);
    nlog(2, &logc, "logged %d and %.1f and %d", 5, 0.5, 5);
    
    deinitlog(&logc);
    return 0;
}