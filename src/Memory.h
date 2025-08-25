#ifndef MEMORY_H
#define MEMORY_H

struct Memory
{
    static void Init();
    static void Term();
    static void* Alloc(size_t Size);
    static void Free(void* Ptr);
};

void* operator new(size_t Size);
void* operator new[](size_t Size);
void operator delete(void* Ptr) noexcept;
void operator delete[](void* Ptr) noexcept;

#endif // MEMORY_H

