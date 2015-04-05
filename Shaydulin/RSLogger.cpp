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

void ERROR(FILE* mysource, string errorMessage){
    fputs("ERROR: ", mysource);
    fputs(errorMessage.c_str(), mysource);
    fputs("\n", mysource);
}

void WARNING(FILE* mysource, string myError){
    if (level >= RSDEBUG){
        fputs( "WARNING: ", mysource);
        fputs( myError.c_str(), mysource);
        fputs("\n", mysource);
    }
}

void INFO(FILE* mysource, string myError){
    if (level >= RSALL){
        fputs( "INFO: ", mysource);
        fputs(myError.c_str(), mysource);
        fputs( "\n", mysource);
    }
}