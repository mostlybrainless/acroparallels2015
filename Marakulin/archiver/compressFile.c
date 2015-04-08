#include "compressFile.h"

// Big file we divide into several small
#define BLOCK_SIZE 1048576

int compressFile(int fdIn, int fdOut)
{
    struct stat inStat;
    fstat(fdIn, &inStat);
    // For output file
    PrefixOfFile pF;
    pF.st_mode = inStat.st_mode;
    pF.st_size = inStat.st_size;
    pF.numbBlocks = 0;
    pF.offsetNextBlock = 0;
    // Properties for dividing into blocks
    off_t restToCompress = inStat.st_size;
    uLongf destLen = 0;
    uLongf destLenApprox = 0;
    off_t offsetNextBlock = 0;

    const Bytef* addr = (const Bytef*)mmap(NULL, pF.st_size, PROT_READ, MAP_PRIVATE, fdIn, 0);
    // First Block
    destLenApprox = compressBound( ((restToCompress < BLOCK_SIZE) ? restToCompress : BLOCK_SIZE) );
    Bytef* outAddr = (Bytef*)calloc(destLenApprox+sizeof(PrefixOfFile), sizeof(Bytef));
    destLen = destLenApprox;
    if (Z_OK != compress(outAddr+sizeof(PrefixOfFile), &destLen, addr, (restToCompress < BLOCK_SIZE) ? restToCompress : BLOCK_SIZE ))
    {
        LOG(ERROR, "Error in compress %d block\n", pF.numbBlocks);
        return -1;
    }
    pF.numbBlocks++;
    pF.offsetNextBlock = sizeof(PrefixOfFile)+destLen;
    offsetNextBlock = pF.offsetNextBlock;

    memcpy(outAddr, &pF, sizeof(PrefixOfFile));
    write(fdOut, outAddr, destLen+sizeof(PrefixOfFile));
    restToCompress -= BLOCK_SIZE;
    // Other blocks
    while (restToCompress > 0)
    {
        destLen = destLenApprox;
        if (Z_OK != compress(outAddr+sizeof(off_t), &destLen, addr+offsetNextBlock, (restToCompress < BLOCK_SIZE) ? restToCompress : BLOCK_SIZE))
        {
            LOG(ERROR, "Error in compress %d block\n", pF.numbBlocks);
            return -1;
        }
        pF.numbBlocks++;
        offsetNextBlock += (sizeof(off_t)+destLen);
        memcpy(outAddr, &offsetNextBlock, sizeof(off_t));
        write(fdOut, outAddr, destLen+sizeof(off_t));
        restToCompress -= BLOCK_SIZE;
    }
    pF.hash = getHashSum(fdOut);
    LOG(TRACE, "hash = %d", pF.hash);
    lseek(fdOut, 0L, 0);
    write(fdOut, &pF, sizeof(PrefixOfFile));
    LOG(TRACE, "pF.st_mode = %d, pF.st_size = %d", pF.st_mode, pF.st_size);
    free(outAddr);
    munmap((void*)addr, pF.st_size);
    return 0;
}

int uncompressFile(int fdIn, char* outFile)
{
    struct stat inStat;
    fstat(fdIn, &inStat);
    LOG(TRACE, "inStat.st_size = %d", inStat.st_size);
    PrefixOfFile pF;
    off_t offsetNextBlock = 0;
    off_t prevOffsetNextBlock = 0;
    const Bytef* addr = (const Bytef*)mmap(NULL, inStat.st_size, PROT_READ, MAP_PRIVATE, fdIn, 0);
    memcpy(&pF, addr, sizeof(PrefixOfFile));
    if (pF.hash != getHashSum(fdIn))
    {
        printf("Don't lie me, it isn't .arc file!\n");
        return -1;
    }
    LOG(TRACE, "pF.st_mode = %d, pF.st_size = %d", pF.st_mode, pF.st_size);
    int fdOut = open(outFile, O_CREAT | O_WRONLY, pF.st_mode);
    if (fdOut == -1)
    {
        munmap((void*)addr, inStat.st_size-sizeof(PrefixOfFile));
        return -1;
    }

    offsetNextBlock = pF.offsetNextBlock;
    prevOffsetNextBlock = sizeof(PrefixOfFile)-sizeof(off_t); // It is the difference in the beginning of the first and subsequent blocks
    uLongf destLen = 0;
    destLen = BLOCK_SIZE;
    Bytef* outAddr = (Bytef*)calloc(((pF.st_size < BLOCK_SIZE) ? pF.st_size : BLOCK_SIZE), sizeof(Bytef));
    int curBlock = 1;

    while (curBlock <= pF.numbBlocks)
    {
        if (pF.numbBlocks == 1)
            destLen = (pF.st_size % (int)BLOCK_SIZE);
        if (Z_OK != uncompress(outAddr, &destLen, addr+sizeof(off_t)+prevOffsetNextBlock, 
            ((curBlock == 1) ? (offsetNextBlock-sizeof(PrefixOfFile)) : (offsetNextBlock-prevOffsetNextBlock)) ))
        {
            LOG(ERROR, "error with decompress %d block\n", curBlock);
            return -1;
        }
        write(fdOut, outAddr, destLen);
        curBlock++;
        prevOffsetNextBlock = offsetNextBlock;
        if(curBlock <= pF.numbBlocks)
            memcpy(&offsetNextBlock, addr+offsetNextBlock, sizeof(off_t));
    }
    close(fdOut);
    munmap((void*)addr, inStat.st_size);
    free(outAddr);
    return 0;
}
