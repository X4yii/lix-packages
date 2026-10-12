#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <lix/syscall.h>

extern int64_t __lix_syscall2(uint64_t n, uint64_t a1, uint64_t a2);
extern int64_t __lix_syscall3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3);
extern int64_t __lix_syscall4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4);
extern int64_t __lix_syscall5(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    (void)offset;
    int64_t ret = __lix_syscall5(SYS_MMAP, (uint64_t)addr, (uint64_t)length, (uint64_t)prot, (uint64_t)flags, (uint64_t)fd);
    if (ret < 0) {
        errno = (int)(-ret);
        return MAP_FAILED;
    }
    return (void *)ret;
}

int munmap(void *addr, size_t length) {
    int64_t ret = __lix_syscall2(SYS_MUNMAP, (uint64_t)addr, (uint64_t)length);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

int mprotect(void *addr, size_t len, int prot) {
    int64_t ret = __lix_syscall3(SYS_MPROTECT, (uint64_t)addr, (uint64_t)len, (uint64_t)prot);
    if (ret < 0) {
        errno = (int)(-ret);
        return -1;
    }
    return 0;
}

typedef struct BlockHeader {
    size_t size;
    struct BlockHeader *next;
    int free;
} BlockHeader;

static BlockHeader *free_list = NULL;

void *malloc(size_t size) {
    if (size == 0) return NULL;
    size_t total_size = ((size + sizeof(BlockHeader) + 15) & ~15);
    BlockHeader *curr = free_list;
    BlockHeader *prev = NULL;
    while (curr) {
        if (curr->free && curr->size >= total_size) {
            curr->free = 0;
            return (void *)(curr + 1);
        }
        prev = curr;
        curr = curr->next;
    }
    size_t alloc_size = total_size < 65536 ? 65536 : total_size;
    void *mem = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        return NULL;
    }
    BlockHeader *header = (BlockHeader *)mem;
    header->size = alloc_size;
    header->free = 0;
    header->next = NULL;
    if (prev) {
        prev->next = header;
    } else {
        free_list = header;
    }
    return (void *)(header + 1);
}

void free(void *ptr) {
    if (!ptr) return;
    BlockHeader *header = ((BlockHeader *)ptr) - 1;
    header->free = 1;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    BlockHeader *header = ((BlockHeader *)ptr) - 1;
    if (header->size >= size + sizeof(BlockHeader)) {
        return ptr;
    }
    void *new_ptr = malloc(size);
    if (!new_ptr) return NULL;
    size_t copy_size = header->size - sizeof(BlockHeader);
    if (copy_size > size) copy_size = size;
    memcpy(new_ptr, ptr, copy_size);
    free(ptr);
    return new_ptr;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *ptr = malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    if ((alignment % sizeof(void *) != 0) || (alignment & (alignment - 1)) != 0) {
        return EINVAL;
    }
    void *ptr = malloc(size);
    if (!ptr) return ENOMEM;
    *memptr = ptr;
    return 0;
}
