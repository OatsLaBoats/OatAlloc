#ifndef INCLUDED_OAT_POOL_ALLOCATOR_H
#define INCLUDED_OAT_POOL_ALLOCATOR_H

#include <stdint.h>

#ifndef OAT_STATIC
#define OAT_STATIC static
#endif

typedef struct OAT_PoolBlock
{
    struct OAT_PoolBlock *next;
} OAT_PoolBlock;

typedef struct OAT_PoolAllocator
{
    uint8_t *memory;
    OAT_PoolBlock *freeBlock;
    uint64_t size;
    uint64_t count;
    uint32_t poolSize;
} OAT_PoolAllocator;

OAT_STATIC void OAT_PoolInit(OAT_PoolAllocator *allocator, void *memory, uint64_t size, uint32_t poolSize, uint32_t alignment);
OAT_STATIC void *OAT_PoolAllocate(OAT_PoolAllocator *allocator);
OAT_STATIC void *OAT_PoolAllocateClean(OAT_PoolAllocator *allocator);
OAT_STATIC void OAT_PoolFree(OAT_PoolAllocator *allocator, void *memory);
OAT_STATIC void OAT_PoolFreeAll(OAT_PoolAllocator *allocator);

#ifdef OAT_POOL_ALLOCATOR_IMPL
    #include <string.h>

    #ifndef PRIVATE_OAT_ALIGN_SIZE
    #define PRIVATE_OAT_ALIGN_SIZE
        static uint64_t private_OAT_AlignSize(uint64_t size, uint32_t alignment) 
        { 
            return (size + alignment - 1) & ~(alignment - 1); 
        }
    #endif

    OAT_STATIC void OAT_PoolInit(OAT_PoolAllocator *allocator, void *memory, uint64_t size, uint32_t poolSize, uint32_t alignment)
    {
        if(poolSize < sizeof(OAT_PoolBlock))
        {
            poolSize = sizeof(OAT_PoolBlock);
        }

        allocator->memory = memory;
        allocator->freeBlock = (OAT_PoolBlock *)allocator->memory;
        allocator->freeBlock->next = NULL;
        allocator->size = size;
        allocator->count = 0;
        allocator->poolSize = private_OAT_AlignSize(poolSize, alignment);
    }

    OAT_STATIC void *OAT_PoolAllocate(OAT_PoolAllocator *allocator)
    {
        uint32_t size = allocator->poolSize;
        
        if(size + allocator->count > allocator->size)
        {
            return NULL;
        }

        void *result = allocator->freeBlock;
        if(allocator->freeBlock->next == NULL)
        {
            //Advance the free block
            allocator->freeBlock = (OAT_PoolBlock *)((uint8_t *)allocator->freeBlock + allocator->poolSize);

            if((uint8_t *)allocator->freeBlock < allocator->memory + allocator->count)
            {
                //Prepare the next free block
                allocator->freeBlock->next = NULL; 
            }
        }
        else
        {
            allocator->freeBlock = allocator->freeBlock->next;
        }

        allocator->count += allocator->poolSize;

        return result;
    }

    OAT_STATIC void *OAT_PoolAllocateClean(OAT_PoolAllocator *allocator)
    {
        void *result = OAT_PoolAllocate(allocator);
        if(result == NULL)
        {
            return NULL;
        }

        memset(result, 0, allocator->poolSize);
        return result;
    }

    OAT_STATIC void OAT_PoolFree(OAT_PoolAllocator *allocator, void *memory)
    {
        OAT_PoolBlock *block = (OAT_PoolBlock *)memory;
        block->next = allocator->freeBlock;
        allocator->freeBlock = block;
        allocator->count -= allocator->poolSize;
    }

    OAT_STATIC void OAT_PoolFreeAll(OAT_PoolAllocator *allocator)
    {
        allocator->freeBlock = (OAT_PoolBlock *)allocator->memory;
        allocator->freeBlock->next = NULL;
        allocator->count = 0;
    }
#endif

#endif