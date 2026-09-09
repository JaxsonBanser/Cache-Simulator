# Heap Memory Allocator

A custom dynamic memory allocator written in C that simulates core heap-management operations such as memory allocation, freeing, block splitting, and coalescing.

The project implements functionality similar to simplified versions of `malloc()` and `free()` while directly managing block metadata and memory layout.

## Features

* Dynamic heap initialization using memory mapping
* Custom memory allocation and deallocation
* Best-fit allocation policy
* 8-byte memory alignment
* Heap block splitting
* Immediate coalescing of adjacent free blocks
* Block headers containing size and allocation metadata
* Footers for free blocks
* Validation of pointers before deallocation
* Heap visualization and debugging output

## How It Works

The allocator manages a contiguous region of memory as a sequence of allocated and free blocks.

Each block contains a header that stores:

* The size of the block
* Whether the current block is allocated
* Whether the previous block is allocated

The two least-significant bits of the block's size field are used to store allocation state while the remaining bits represent the size of the block.

Free blocks additionally contain a footer storing their size. This allows the allocator to efficiently locate the previous block when performing coalescing.

## Memory Allocation

The `balloc()` function acts as the allocator's equivalent of `malloc()`.

When memory is requested, the allocator:

1. Adds space for block metadata.
2. Rounds the total block size to an 8-byte boundary.
3. Searches the heap for available memory.
4. Uses a **best-fit placement policy** to select the smallest free block large enough for the request.
5. Splits the selected block when enough unused space remains.
6. Updates the metadata of neighboring blocks.
7. Returns a pointer to the usable payload area.

If no suitable block exists, the allocation fails and returns `NULL`.

## Memory Deallocation

The `bfree()` function acts as the allocator's equivalent of `free()`.

Before freeing memory, the allocator validates the supplied pointer and ensures that:

* The pointer is not `NULL`
* The block is currently allocated
* The block exists within the managed heap
* The block follows the allocator's alignment requirements

Once freed, the allocator updates the block's metadata and attempts to immediately merge adjacent free blocks.

## Coalescing

To reduce external fragmentation, neighboring free blocks are combined into larger blocks whenever possible.

The allocator can coalesce with:

* The next free block
* The previous free block
* Both neighboring blocks

Free-block footers allow the allocator to determine the size and location of the previous block without traversing the entire heap.

## Heap Initialization

The heap is initialized through `init_heap()`.

The allocator:

* Determines the system page size
* Rounds the requested heap region to a page boundary
* Creates a memory-mapped region
* Initializes the region as a single free block
* Creates an end marker used when traversing the heap

## Heap Visualization

The `disp_heap()` function can be used to inspect the current state of the allocator.

For each block, it displays information such as:

* Allocation status
* Previous block status
* Starting memory address
* Ending memory address
* Block size

It also reports the total amount of allocated and free memory.

## Key Concepts Demonstrated

This project explores several low-level systems programming concepts, including:

* Dynamic memory management
* Pointer arithmetic
* Memory alignment
* Bit manipulation
* Heap organization
* Internal and external fragmentation
* Best-fit allocation
* Block splitting
* Boundary tags
* Memory coalescing
* Memory-mapped regions

## Tech Stack

* **C**
* POSIX system calls
* `mmap`
* Linux/Unix memory-management APIs

## Example Memory Layout

```text
+--------------+----------------------+--------------+
| Block Header |       Payload        |              |
+--------------+----------------------+--------------+

Allocated Block:
+--------+----------------------+
| Header |       Payload        |
+--------+----------------------+

Free Block:
+--------+----------------------+--------+
| Header |      Free Space      | Footer |
+--------+----------------------+--------+
```

The header stores the block size and allocation status. Free blocks also contain a footer containing the block size, allowing neighboring free blocks to be efficiently merged.

## Purpose

This project was created to better understand how dynamic memory allocation works beneath higher-level functions such as `malloc()` and `free()`.

Implementing a heap allocator from scratch provided hands-on experience with low-level memory manipulation, pointer arithmetic, alignment, metadata encoding, fragmentation, and allocation strategies.
