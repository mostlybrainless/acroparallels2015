//
//  archivator.h
//  archivator
//
//  Created by Daniil Sargin on 08/04/15.
//  Copyright (c) 2015 Даниил Саргин. All rights reserved.
//

#ifndef __archivator__archivator__
#define __archivator__archivator__

#include "logger.h"

log_context LOGC;

int arch_deflate(const char * src_name);
int arch_inflate(const char * src_name);

#endif /* defined(__archivator__archivator__) */
