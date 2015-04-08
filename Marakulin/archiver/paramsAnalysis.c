#include "paramsAnalysis.h"

#define MAX_NUMB_PARAMS 4
#define MIN_NUMB_PARAMS 2

#define PACK "-pack"
#define UNPACK "-unpack"
#define GET_EXAMPLE "-getExample"

#define GET_EXAMPLE_STRINGS() \
do { \
    printf("Example:\n"); \
    printf("archiver -pack ./picture.jpg ./arcPicture.arc\n"); \
    printf("archiver -unpack ./arcPicture.arc ./out.jpg\n"); \
} while(0)

int analyzeParams(int argc, char* argv[])
{
    LOG(TRACE, "argc = %d", argc);
    if ((argc > MAX_NUMB_PARAMS) || (argc < MIN_NUMB_PARAMS))
    {
        printf("Wrong input\n");
        GET_EXAMPLE_STRINGS();
        return -1;
    }
    else
    {
        if (!strcmp(PACK, argv[1]))
        {
            return pack(argc-1, argv+1);
        }
        else if (!strcmp(UNPACK, argv[1]))
        {
            return unpack(argc-1, argv+1);
        }
        else if (!strcmp(GET_EXAMPLE, argv[1]))
        {
            return getExample(argc-1, argv+1);
        }
        else
        {
            printf("Wrong input, check first param\n");
            GET_EXAMPLE_STRINGS();
            return -1;
        }
    }
    return 0;
}

int pack(int argc, char* argv[])
{
    LOG(TRACE, "argc = %d", argc);
    if ((argc != 2) && (argc != 3))
    {
        printf("Wrong input, check number of params\n");
        GET_EXAMPLE_STRINGS();
        return -1;
    }
    LOG(TRACE, "argv[1] = '%s'", argv[1]);
    if (access(argv[1], F_OK) == -1)
    {
        printf("source file doesn't exist\n");
        return -1;
    }
    int fdOut = 0;
    if (argc == 2)
    {
        // 5 fot '.arc'
        char* outFile = calloc(strlen(argv[1])+5, sizeof(char));
        strcpy(outFile, argv[1]);
        strcat(outFile, ".arc");
        LOG(TRACE, "outFile = '%s'", outFile);
        if (access(outFile, F_OK) != -1)
        {
            printf("destination file exists");
            return -1;
        }
        fdOut = open(outFile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        free(outFile);
    }
    else
    {
        LOG(TRACE, "argv[2] = '%s'", argv[2]);
        if (access(argv[2], F_OK) != -1)
        {
            printf("destination file exists\n");
            return -1;
        }
        fdOut = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    }
    if (fdOut == -1)
    {
        printf("error opening destination file\n");
        return -1;
    }
    int fdIn = open(argv[1], O_RDONLY);
    if (fdIn == -1)
    {
        close(fdOut);
        printf("error opening source file\n");
        return -1;
    }
    compressFile(fdIn, fdOut);
    close(fdOut);
    close(fdIn);
    return 0;
}

int unpack(int argc, char* argv[])
{
    LOG(TRACE, "argc = %d");
    if ((argc != 2) && (argc != 3))
    {
        printf("Wrong input, check number of params\n");
        return -1;
    }
    LOG(TRACE, "argv[1] = '%s'", argv[1]);
    if (access(argv[1], R_OK) == -1)
    {
        printf("Can't read source file\n");
        return -1;
    }
    char* outFile;
    if (argc == 2)
    {
        if ((strstr(argv[1], ".arc") != (argv[1] + strlen(argv[1])-4)) || (strlen(argv[1]) < 5))
        {
            printf("wrong name of source file\n");
            return -1;
        }
        outFile = calloc(strlen(argv[1])-3, sizeof(char));
        memcpy(outFile, argv[1], strlen(argv[1]) - 4);
        if (access(outFile, F_OK) != -1)
        {
            printf("destination file exists\n");
            return -1;
        }
    }
    else
    {
        LOG(TRACE, "argv[2] = '%s'", argv[2]);
        if (access(argv[2], F_OK) != -1)
        {
            printf("destination file exists\n");
            return -1;
        }
        outFile = calloc(strlen(argv[2])+1, sizeof(char));
        memcpy(outFile, argv[2], strlen(argv[2]));
    }
    int fdIn = open(argv[1], O_RDONLY);
    if (fdIn == -1)
    {
        free(outFile);
        printf("error opening source file\n");
        return -1;
    }
    LOG(TRACE, "outFile = '%s'", outFile);
    int ret = uncompressFile(fdIn, outFile);
    free(outFile);
    close(fdIn);
    if (ret != 0)
    {
        printf("error with uncompress file\n");
        return -1;
    }
    return 0;
}

int getExample(int argc, char* argv[])
{
    LOG(TRACE, "argc = %d", argc);
    GET_EXAMPLE_STRINGS();
    return 0;
}
