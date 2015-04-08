#include "hashFunc.h"

unsigned int HashFAQ6(const char * str, off_t length)
{
    int hash = 0;
    int i = 0;
    for (i = 0; i < length; i++)
    {
        hash += str[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
        printf("%d\n", i);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

off_t getHashSum(int fd)
{
    struct stat fileStat;
    int ret1 = fstat(fd, &fileStat);
    Bytef* addr = (Bytef*)mmap(NULL, fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    off_t ret = HashFAQ6(addr+sizeof(PrefixOfFile), fileStat.st_size-sizeof(PrefixOfFile));
    munmap((void*)addr, fileStat.st_size);
    return ret;
}
