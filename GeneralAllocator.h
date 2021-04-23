#ifndef INCLUDED_OATS_GENERAL_ALLOCATOR_H
#define INCLUDED_OATS_GENERAL_ALLOCATOR_H

#include <stdint.h>

#ifndef OAT_STATIC
#define OAT_STATIC static
#endif

typedef uint64_t OAT_HeapSize;

typedef struct OAT_HeapBlock
{
    OAT_HeapSize size;
    
    union
    {
        uint8_t data;

        struct
        {
            struct OAT_HeapBlock *prev;
            struct OAT_HeapBlock *next;
        };
    };
} OAT_HeapBlock;

typedef struct OAT_GeneralAllocator
{
    uint8_t *memory;
    OAT_HeapBlock *headBlock;
    OAT_HeapBlock *freeBlock;
    uint64_t size;
    uint64_t count;
    uint32_t alignment;
} OAT_GeneralAllocator;

OAT_STATIC void OAT_GeneralInit(OAT_GeneralAllocator *allocator, void *memory, uint64_t size, uint32_t alignment);
OAT_STATIC void *OAT_GeneralAllocate(OAT_GeneralAllocator *allocator, uint64_t size);
OAT_STATIC void *OAT_GeneralAllocateClean(OAT_GeneralAllocator *allocator, uint64_t size);
OAT_STATIC void OAT_GeneralFree(OAT_GeneralAllocator *allocator, void *memory);
OAT_STATIC void OAT_GeneralFreeAll(OAT_GeneralAllocator *allocator);
OAT_STATIC void *OAT_GeneralReallocate(OAT_GeneralAllocator *allocator, void *memory, uint64_t size);
OAT_STATIC void *OAT_GeneralReallocateClean(OAT_GeneralAllocator *allocator, void *memory, uint64_t size);

#ifdef OAT_GENERAL_ALLOCATOR_IMPL
    #include <string.h>

    #define PRIVATE_OAT_MIN_ALLOC_SIZE (sizeof(OAT_HeapBlock *) * 2)

    #ifndef PRIVATE_OAT_ALIGN_SIZE
    #define PRIVATE_OAT_ALIGN_SIZE
        static uint64_t private_OAT_AlignSize(uint64_t size, uint32_t alignment) 
        { 
            return (size + alignment - 1) & ~(alignment - 1); 
        }
    #endif

    static OAT_HeapBlock *private_OAT_FindFreeHeapBlock(OAT_GeneralAllocator *allocator, uint32_t size)
    {
        OAT_HeapBlock *block = allocator->freeBlock;
        while(block != NULL)
        {
            if(block->size >= size)
            {
                return block;
            }

            block = block->next;
        }

        return NULL;
    }

    static inline OAT_HeapBlock *private_OAT_GetHeapBlockFromPtr(void *ptr)
    {
        return (OAT_HeapBlock *)((uint8_t *)ptr - sizeof(OAT_HeapSize));
    }

    OAT_STATIC void OAT_GeneralInit(OAT_GeneralAllocator *allocator, void *memory, uint64_t size, uint32_t alignment)
    {
        allocator->memory = memory;
        allocator->size = size;
        allocator->count = 0;
        allocator->headBlock = (OAT_HeapBlock *)memory;
        allocator->freeBlock = NULL;
        allocator->alignment = alignment;
    }

    OAT_STATIC void *OAT_GeneralAllocate(OAT_GeneralAllocator *allocator, uint64_t size)
    {
        size = private_OAT_AlignSize(size, allocator->alignment);
        if(size < PRIVATE_OAT_MIN_ALLOC_SIZE)
        {
            size = PRIVATE_OAT_MIN_ALLOC_SIZE;
        }

        uint32_t realSize = size + sizeof(OAT_HeapSize);

        if(realSize + allocator->count > allocator->size)
        {
            return NULL;
        }

        OAT_HeapBlock *block = private_OAT_FindFreeHeapBlock(allocator, size);
        if(block == NULL)
        {
            block = allocator->headBlock;
            block->size = size;
            allocator->count += realSize;
            allocator->headBlock = (OAT_HeapBlock *)((uint8_t *)allocator->headBlock + realSize);
        }
        else
        {
            if(block->size == size || (block->size - size) < sizeof(OAT_HeapBlock))
            {
                if(block->prev != NULL)
                {
                    block->prev->next = block->next;
                }
                else
                {
                    allocator->freeBlock = allocator->freeBlock->next;
                }

                if(block->next != NULL)
                {
                    block->next->prev = block->prev;
                }       

                allocator->count += block->size + sizeof(OAT_HeapSize);
            }
            else
            {
                OAT_HeapBlock *newBlock = (OAT_HeapBlock *)((uint8_t *)block + realSize);

                newBlock->prev = block->prev;
                newBlock->next = block->next;
                newBlock->size = block->size - realSize;

                if(block->prev != NULL)
                {
                    block->prev->next = newBlock;
                }
                else
                {
                    allocator->freeBlock = newBlock;
                }

                if(block->next != NULL)
                {
                    block->next->prev = newBlock;
                }
            }
        }

        return &block->data;
    }

    OAT_STATIC void *OAT_GeneralAllocateClean(OAT_GeneralAllocator *allocator, uint64_t size)
    {
        void *result = OAT_GeneralAllocate(allocator, size);
        if(result == NULL)
        {
            return NULL;
        }

        memset(result, 0, size);
        return result;
    }

    OAT_STATIC void OAT_GeneralFree(OAT_GeneralAllocator *allocator, void *memory)
    {
        OAT_HeapBlock *block = private_OAT_GetHeapBlockFromPtr(memory);

        block->next = allocator->freeBlock;
        block->prev = NULL;
        if(block->next != NULL)
        {
            block->next->prev = block;
        }

        allocator->freeBlock = block;
        allocator->count -= block->size + sizeof(OAT_HeapSize);
    }

    OAT_STATIC void OAT_GeneralFreeAll(OAT_GeneralAllocator *allocator)
    {
        allocator->count = 0;
        allocator->freeBlock = NULL;
        allocator->headBlock = (OAT_HeapBlock *)allocator->memory;
    }

    OAT_STATIC void *OAT_GeneralReallocate(OAT_GeneralAllocator *allocator, void *memory, uint64_t size)
    {
        OAT_HeapBlock *block = private_OAT_GetHeapBlockFromPtr(memory);
        void *result = OAT_GeneralAllocate(allocator, size);
        
        memcpy(result, memory, block->size);
        OAT_GeneralFree(allocator, memory);
        
        return result;
    }

    OAT_STATIC void *OAT_GeneralReallocateClean(OAT_GeneralAllocator *allocator, void *memory, uint64_t size)
    {
        OAT_HeapBlock *block = private_OAT_GetHeapBlockFromPtr(memory);
        void *result = OAT_GeneralAllocateClean(allocator, size);
        
        memcpy(result, memory, block->size);
        OAT_GeneralFree(allocator, memory);
        
        return result;
    }
#endif

#endif