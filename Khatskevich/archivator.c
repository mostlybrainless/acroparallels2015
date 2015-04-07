#define DEBUG
#include "archivator.h"

#include <stdio.h>
#include <stdlib.h>


static char* PREFIX_FOR_number_of_scaners = "-ns";
static char* PREFIX_FOR_show_help = "-h";
static char* PREFIX_FOR_number_of_workers = "-nw";
static char* PREFIX_FOR_number_of_processes = "-np";
static char* PREFIX_FOR_compression_lvl = "-lvl";
static char* PREFIX_FOR_out_f_name = "-of";
static char* PREFIX_FOR_decompress = "-d";

void printHelp(){
            printf("For control processes number, use prefixes:\n\tnumber of scaners %s\n\tnumber of compressors %s\n\tset borth of previous %s\n", 
                   PREFIX_FOR_number_of_scaners ,PREFIX_FOR_number_of_workers,PREFIX_FOR_number_of_processes);
            printf("To set compression lvl, use prefix: %s    (default = 3)\n", 
                   PREFIX_FOR_compression_lvl);
            printf("To set out archive/uncompressed folder name, use prefix: %s\n", 
                   PREFIX_FOR_out_f_name);
};

int main(int argc, char** argv){
    logInit(LOG_ERROR , LOG_PRINT_TIME | LOG_PRINT_LEVEL_DESCRIPTION | LOG_PRINT_FILE | LOG_PRINT_LINE , NULL);
    int i;
    int number_of_scaners=0;
    int number_of_workers=0;
    int compression_lvl=3;
    char* out_f_name = NULL;
    char* files[MAXIMAL_INPUT_FILES+1];
    int current_number_of_input_files = 0;
    int decompress = 0;
    
    for (i = 1; i < argc; i++)  /* Skip argv[0] (program name). */
    {
        if ( (strcmp(argv[i], PREFIX_FOR_number_of_scaners) == 0) && i+1<argc )  /* Process optional arguments. */
        {
            sscanf(argv[++i],"%d",&number_of_scaners );
            LOGMESG(LOG_INFO, "number_of_scaners = %d", number_of_scaners);
            continue;
        }
        if ( (strcmp(argv[i], PREFIX_FOR_number_of_workers) == 0) && i+1<argc )  /* Process optional arguments. */
        {
            sscanf(argv[++i],"%d",&number_of_workers );
            LOGMESG(LOG_INFO, "number_of_workers = %d", number_of_workers);
            continue;
        }
        if ( (strcmp(argv[i], PREFIX_FOR_number_of_processes) == 0) && i+1<argc )  /* Process optional arguments. */
        {
            sscanf(argv[++i],"%d",&number_of_workers );
            number_of_scaners = number_of_workers;
            continue;
        }
        if ( (strcmp(argv[i], PREFIX_FOR_compression_lvl) == 0) && i+1<argc )  /* Process optional arguments. */
        {
            sscanf(argv[++i],"%d",&compression_lvl );
            continue;
        }
        if ( strcmp(argv[i], PREFIX_FOR_decompress) == 0 )  /* Process optional arguments. */
        {
            decompress = 1;
            continue;
        }
        if ( (strcmp(argv[i], PREFIX_FOR_out_f_name) == 0) && i+1<argc )  /* Process optional arguments. */
        {
            out_f_name = argv[++i] ;
            LOGMESG(LOG_INFO, "Output file name: %s", out_f_name);
            continue;
        }
        if ( strcmp(argv[i], PREFIX_FOR_show_help) == 0 ){
            printHelp();
            continue;
        } 
        if ( current_number_of_input_files < MAXIMAL_INPUT_FILES){
            files[current_number_of_input_files++] = argv[i];
            LOGMESG(LOG_INFO, "File process: %s",  files[current_number_of_input_files-1]);
            continue;
        }
    }
    files[current_number_of_input_files] = NULL;
    if ( number_of_scaners == 0){
        number_of_scaners = NUMBER_OF_SCANERS;
    }
    if ( number_of_workers == 0){
        number_of_workers = NUMBER_OF_WORKERS;
    }
    
    if ( decompress == 1 && current_number_of_input_files > 0 ){
        LOGMESG(LOG_INFO, "Decompression...");
        char* out_folder_name = NULL;
        if (out_f_name == NULL){
            out_folder_name = (char*) malloc( sizeof(files[0]) + sizeof("_rst") + 1 );
            sprintf( out_folder_name, "%s%s", files[0], "_rst" );
        }else{
            out_folder_name = (char*) malloc( sizeof(out_f_name) + 1 );
            sprintf( out_folder_name, "%s", out_f_name);
        }
        dataRestore( out_folder_name ,files[0]);
        free(out_folder_name);
    }else if (decompress == 0 && out_f_name != NULL){
        LOGMESG(LOG_INFO, "Compression...");
        dataPresentationControllerInit(out_f_name, compression_lvl , number_of_scaners, number_of_workers, files, argv[0] );
        compressionPerform();
    }else{
        LOGMESG(LOG_ERROR, "Invalid arguments");
        printHelp();
    }
    logClose();
    
    return 0;
}
 
