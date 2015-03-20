//
//  RSLogger.cpp
//  myLogger
//
//  Created by Руслан on 16.03.15.
//
//

#include "RSLogger.h"

int level = RSALL;

void setLogLevel(int newlevel){
    level = newlevel;
}

void ERROR(FILE* mysource, char errorMessage[]){
    fprintf(mysource, "ERROR: ");
    fprintf(mysource, errorMessage);
    fprintf(mysource, "\n");
}

void WARNING(FILE* mysource, char myError[]){
    if (level >= RSDEBUG){
        fprintf(mysource, "WARNING: ");
        fprintf(mysource, myError);
        fprintf(mysource, "\n");
    }
}

void INFO(FILE* mysource, char myError[]){
    if (level >= RSALL){
        fprintf(mysource, "INFO: ");
        fprintf(mysource, myError);
        fprintf(mysource, "\n");
    }
}