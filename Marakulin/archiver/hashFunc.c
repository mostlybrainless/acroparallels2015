#include "hashFunc.h"

#define MAX 262144

off_t HashFAQ6(const char * str, off_t length)
{
    off_t hash = 0;
    off_t i = 0;
    for (i = 0; i < length; i++)
    {
        if (hash > MAX)
            hash = hash % MAX;
        hash += str[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

off_t getHashSum(int fd)
{
    struct stat fileStat;
    fstat(fd, &fileStat);
    const char* addr = (const char*)mmap(NULL, fileStat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    off_t ret = HashFAQ6(addr+sizeof(PrefixOfFile), fileStat.st_size-sizeof(PrefixOfFile));
    munmap((void*)addr, fileStat.st_size);
    return ret;
}
