#include "UBG.h"

struct MemBlock
{
    size_t Size;
    void* Data;

    void* GetBlockEnd()
    {
        return (void*)((u8*)Data + Size);
    }
};


struct MemPool
{
    static constexpr size_t MaxBlocks = 1024;

    size_t DataSize;
    void* DataPool;

    size_t NumAlloc;
    MemBlock AllocBlocks[MaxBlocks];
    size_t NumFree;
    MemBlock FreeBlocks[MaxBlocks];

    void Init(size_t Size)
    {
        ASSERT(!DataSize);
        ASSERT(!DataPool);
        ASSERT(!NumAlloc);
        ASSERT(!NumFree);

        DataSize = Size;
        DataPool = malloc(Size);

        NumAlloc = 0;
        NumFree = 1;
        FreeBlocks[0] = { Size, DataPool };
    }

    void Term()
    {
        ASSERT(DataSize && DataPool);

        DataSize = 0;
        free(DataPool);
        DataPool = nullptr;

        NumAlloc = 0;
        NumFree = 0;
        memset(AllocBlocks, 0, sizeof(AllocBlocks));
        memset(FreeBlocks, 0, sizeof(FreeBlocks));
    }

    void* Alloc(size_t Size)
    {
        ASSERT(DataSize && DataPool);

        int FoundIdx = -1;
        for (int FreeIdx = 0; FreeIdx < NumFree; FreeIdx++)
        {
            if (FreeBlocks[FreeIdx].Size >= Size)
            {
                FoundIdx = FreeIdx;
                break;
            }
        }
        ASSERT(FoundIdx != -1);
        if (FoundIdx >= 0)
        {
            MemBlock NewAlloc = {};
            if (FreeBlocks[FoundIdx].Size == Size)
            {
                NewAlloc = FreeBlocks[FoundIdx];
                for (int Idx = FoundIdx; (Idx + 1) < NumFree; Idx++)
                {
                    FreeBlocks[Idx] = FreeBlocks[Idx + 1];
                }
                FreeBlocks[--NumFree] = {};
            }
            else if (FreeBlocks[FoundIdx].Size > Size)
            {
                NewAlloc = { Size, FreeBlocks[FoundIdx].Data };
                FreeBlocks[FoundIdx].Data = (u8*)FreeBlocks[FoundIdx].Data + Size;
                FreeBlocks[FoundIdx].Size -= Size;
            }
            ASSERT(NewAlloc.Data && NewAlloc.Size == Size);

            // Case 1: NewAlloc is the first alloc
            if (NumAlloc == 0)
            {
                AllocBlocks[0] = NewAlloc;
                NumAlloc++;
            }
            // Case 2: NewAlloc comes before the first alloc
            else if (NumAlloc > 0 && NewAlloc.Data < AllocBlocks[0].Data)
            {
                for (int ShiftUpIdx = NumAlloc; ShiftUpIdx > 0; ShiftUpIdx--)
                {
                    AllocBlocks[ShiftUpIdx] = AllocBlocks[ShiftUpIdx - 1];
                }
                AllocBlocks[0] = NewAlloc;
                NumAlloc++;
            }
            // Case 3: NewAlloc comes after last alloc
            else if (NumAlloc > 0 && NewAlloc.Data > AllocBlocks[NumAlloc - 1].Data)
            {
                AllocBlocks[NumAlloc++] = NewAlloc;
            }
            // Case 4: NewAlloc comes in the middle of other allocs
            else
            {
                int NewAllocIdx = -1;
                for (int Idx = 0; (Idx + 1) < NumAlloc; Idx++)
                {
                    if (AllocBlocks[Idx].Data < NewAlloc.Data &&
                        AllocBlocks[Idx + 1].Data < NewAlloc.Data)
                    {
                        NewAllocIdx = Idx + 1;
                    }
                }
                ASSERT(NewAllocIdx >= 0);
                if (NewAllocIdx >= 0)
                {
                    for (int ShiftUpIdx = NumAlloc; ShiftUpIdx > NewAllocIdx; ShiftUpIdx--)
                    {
                        AllocBlocks[ShiftUpIdx] = AllocBlocks[ShiftUpIdx - 1];
                    }
                    AllocBlocks[NewAllocIdx] = NewAlloc;
                    NumAlloc++;
                }
            }

            return NewAlloc.Data;
        }
        else
        {
            return nullptr;
        }
    }

    void Free(void* Ptr)
    {
        ASSERT(DataSize && DataPool);

        int FoundIdx = -1;
        for (int Idx = 0; Idx < NumAlloc; Idx++)
        {
            if (AllocBlocks[Idx].Data == Ptr)
            {
                FoundIdx = Idx;
                break;
            }
        }
        ASSERT(FoundIdx >= 0);

        if (FoundIdx >= 0)
        {
            MemBlock NewFree = AllocBlocks[FoundIdx];
            for (int Idx = FoundIdx; (Idx + 1) < NumAlloc; Idx++)
            {
                AllocBlocks[Idx] = AllocBlocks[Idx + 1];
            }

            // Check if NewFree can be coalesced (is adjacent to other free blocks)
            bool bCoalesced = false;
            int NewFreeIdx = -1;

            for (int Idx = 0; (Idx + 1) < NumFree; Idx++)
            {
                bool bFreeBefore = FreeBlocks[Idx].GetBlockEnd() == NewFree.Data;
                bool bFreeAfter = NewFree.GetBlockEnd() == FreeBlocks[Idx + 1].Data;

                // Case 1: NewFree is between two free blocks
                if (bFreeBefore && bFreeAfter)
                {
                    size_t CoalescedSize = FreeBlocks[Idx].Size + NewFree.Size + FreeBlocks[Idx + 1].Size;
                    FreeBlocks[Idx].Size = CoalescedSize;

                    for (int ShiftDownIdx = Idx + 1; (ShiftDownIdx + 1) < NumFree; ShiftDownIdx++)
                    {
                        FreeBlocks[ShiftDownIdx] = FreeBlocks[ShiftDownIdx + 1];
                    }
                    FreeBlocks[--NumFree] = {};

                    bCoalesced = true;
                    break;
                }
                // Case 2: NewFree is after a free block
                else if (bFreeBefore)
                {
                    size_t CoalescedSize = FreeBlocks[Idx].Size + NewFree.Size;
                    FreeBlocks[Idx].Size = CoalescedSize;

                    bCoalesced = true;
                    break;
                }
                // Case 3: NewFree is before a free block
                else if (bFreeAfter)
                {
                    size_t CoalescedSize = FreeBlocks[Idx + 1].Size + NewFree.Size;
                    FreeBlocks[Idx].Data = NewFree.Data;
                    FreeBlocks[Idx].Size = CoalescedSize;

                    bCoalesced = true;
                    break;
                }

                // Case 4: NewFree is between two alloc blocks
                if (!bCoalesced &&
                    ((u8*)FreeBlocks[Idx].Data < (u8*)NewFree.Data) &&
                    ((u8*)NewFree.Data < (u8*)FreeBlocks[Idx + 1].Data))
                {
                    NewFreeIdx = Idx + 1;
                    break;
                }
            }

            // Case 5: NewFree is first or last free block
            if (!bCoalesced && NewFreeIdx == -1)
            {
                bool bComesFirst = (u8*)NewFree.Data < (u8*)FreeBlocks[0].Data;
                bool bComesLast = (u8*)FreeBlocks[NumFree - 1].Data < (u8*)NewFree.Data;
                ASSERT(bComesFirst != bComesLast);

                if (bComesFirst)
                {
                    for (int ShiftUpIdx = NumFree; ShiftUpIdx > 0; ShiftUpIdx--)
                    {
                        FreeBlocks[ShiftUpIdx] = FreeBlocks[ShiftUpIdx - 1];
                    }
                    FreeBlocks[0] = NewFree;
                    NumFree++;
                }
                else if (bComesLast)
                {
                    FreeBlocks[NumFree++] = NewFree;
                }
                else
                {
                    ASSERT(false); // TODO: Remove once tested
                }
            }
            else if (!bCoalesced && NewFreeIdx >= 0)
            {
                for (int ShiftUpIdx = NumFree; ShiftUpIdx > NewFreeIdx; ShiftUpIdx++)
                {
                    FreeBlocks[ShiftUpIdx] = FreeBlocks[ShiftUpIdx - 1];
                }
                FreeBlocks[NewFreeIdx] = NewFree;
                NumFree++;
            }
            else
            {
                ASSERT(false); // TODO: Remove once tested
            }
        }
        else
        {
            ASSERT(false);
        }
    }
};

struct MemoryImpl
{
    static constexpr size_t TransientSize = 1024ull * 1024ull * 1024ull;

    static bool bInit;
    static MemPool Transient;

    static void Init()
    {
        ASSERT(!bInit);

        Transient.Init(TransientSize);

        bInit = true;
    }
    static void Term()
    {
        ASSERT(bInit);

        Transient.Term();

        bInit = false;
    }

    static void* Alloc(size_t Size)
    {
        ASSERT(bInit);
        return Transient.Alloc(Size);
    }
    static void Free(void* Ptr)
    {
        ASSERT(bInit);
        Transient.Free(Ptr);
    }
};

bool MemoryImpl::bInit = {};
MemPool MemoryImpl::Transient = {};

void Memory::Init()
{
    MemoryImpl::Init();
}

void Memory::Term()
{
    MemoryImpl::Term();
}

void* Memory::Alloc(size_t Size)
{
    return MemoryImpl::Alloc(Size);
}

void Memory::Free(void* Ptr)
{
    MemoryImpl::Free(Ptr);
}

void* operator new(size_t Size)
{
    return Memory::Alloc(Size);
}

void* operator new[](size_t Size)
{
    return Memory::Alloc(Size);
}

void operator delete(void* Ptr) noexcept
{
    if (Ptr) { Memory::Free(Ptr); }
}

void operator delete[](void* Ptr) noexcept
{
    if (Ptr) { Memory::Free(Ptr); }
}

