#ifndef INCLUDED_OAT_BUMP_ALLOCATOR_H
#define INCLUDED_OAT_BUMP_ALLOCATOR_H

#include <stdint.h>

#ifndef OAT_STATIC
#define OAT_STATIC static
#endif

typedef struct OAT_BumpAllocator
{
    uint8_t *memory;
    uint8_t *nextAlloc;
    uint64_t size;
    uint64_t count;
    uint32_t alignment;
} OAT_BumpAllocator;

OAT_STATIC void OAT_BumpInit(OAT_BumpAllocator *allocator, void *memory, uint64_t size, uint32_t alignment);
OAT_STATIC void *OAT_BumpAllocate(OAT_BumpAllocator *allocator, uint64_t size);
OAT_STATIC void *OAT_BumpAllocateClean(OAT_BumpAllocator *allocator, uint64_t size);
OAT_STATIC void OAT_BumpFree(OAT_BumpAllocator *allocator, void *memory);
OAT_STATIC void OAT_BumpFreeAll(OAT_BumpAllocator *allocator);

//Implementation
#ifdef OAT_BUMP_ALLOCATOR_IMPL
    #include <string.h>

    #ifndef PRIVATE_OAT_ALIGN_SIZE
    #define PRIVATE_OAT_ALIGN_SIZE
        static uint64_t private_OAT_AlignSize(uint64_t size, uint32_t alignment) 
        { 
            return (size + alignment - 1) & ~(alignment - 1); 
        }
    #endif

    OAT_STATIC void OAT_BumpInit(OAT_BumpAllocator *allocator, void *memory, uint64_t size, uint32_t alignment)
    {
        allocator->memory = memory;
        allocator->nextAlloc = memory;
        allocator->size = size;
        allocator->count = 0;
        allocator->alignment = alignment;
    }

    OAT_STATIC void *OAT_BumpAllocate(OAT_BumpAllocator *allocator, uint64_t size)
    {
        size = private_OAT_AlignSize(size, allocator->alignment);

        if(size + allocator->count > allocator->size)
        {
            return NULL;
        }

        void *result = allocator->nextAlloc;
        allocator->nextAlloc += size;
        allocator->count += size;

        return result;
    }

    OAT_STATIC void *OAT_BumpAllocateClean(OAT_BumpAllocator *allocator, uint64_t size)
    {
        void *result = OAT_BumpAllocate(allocator, size);
        if(result == NULL)
        {
            return NULL;
        }

        memset(result, 0, size);
        return result;
    }

    OAT_STATIC void OAT_BumpFree(OAT_BumpAllocator *allocator, void *memory)
    {
        uint64_t byteCount = (uint64_t)allocator->nextAlloc - (uint64_t)memory;
        allocator->nextAlloc = memory;
        allocator->count -= byteCount;
    }

    OAT_STATIC void OAT_BumpFreeAll(OAT_BumpAllocator *allocator)
    {
        allocator->count = 0;
        allocator->nextAlloc = allocator->memory;
    }
#endif

#endif