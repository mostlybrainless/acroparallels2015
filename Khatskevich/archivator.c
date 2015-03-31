#define DEBUG
#include "log.h"
#include "data_presentation.h"
#include "scaner.h"
#include <stdio.h>
#include <stdlib.h>
#include <linux/ipc.h>
#include <linux/msg.h>

int main(int argc, char** argv){
    logInit(LOG_INFO, LOG_PRINT_LEVEL_DESCRIPTION | LOG_PRINT_FILE | LOG_PRINT_LINE , NULL);
    char* files[2];
    files[0] = argv[1];
    files[1] = NULL; 
    dataPresentationControllerInit("valera.txt", 3, 1, files, argv[0] );
    compressionPerform();
    logClose();
}
 
