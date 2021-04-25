# OatAlloc

Single file header only allocators.

Contains:

- Pool allocator
- General allocator
- Bump allocator

## Pool allocator

The pool allocator allocates only one size specified at initialization. It can reuse free memory and is very fast O(1).

## General allocator

The general allocator is essentialy a simple unoptimized implementation of malloc.

## Bump allocator

The bump allocator or otherwise known as stack allocator allocates memory by incerementing a pointer with each allocation. Free can should only be used for the last allocation or else it will free more than the desired memory.

## Notes

Make sure to define the implementation macro before including the header file.

``` C
#define OAT_BUMP_ALLOCATOR_IMPL
#include "BumpAllocator.h"

#define OAT_POOL_ALLOCATOR_IMPL
#include "PoolAllocator.h"

#define OAT_GENERAL_ALLOCATOR_IMPL
#include "GeneralAllocator.h"
```
