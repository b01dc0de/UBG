#include "UBG.h" // Includes Memory.h

struct MemBlock
{
    // NOTE:
    //  - We're losing 7 bytes of data from packing here
    //  - What other information would be useful here?
    //  - We could also pack info into the lowest 2 bytes of either Size _or_ Data since we enforce 16-bit alignment
    bool bAlloc = false;
    size_t Size = 0;
    void* Data = nullptr;
};

struct MemPoolPersistent
{
    size_t PoolSize = 0;
    void* PoolData = nullptr;
    size_t BytesAllocated = 0;

    void Init(size_t Size)
    {
        ASSERT(!PoolSize && !PoolData && !BytesAllocated);

        PoolSize = Size;
        PoolData = malloc(Size);
        BytesAllocated = 0;
    }
    void Term()
    {
        ASSERT(PoolSize && PoolData);

        PoolSize = 0;
        free(PoolData);
        BytesAllocated = 0;
    }

    void* Alloc(size_t Size)
    {
        ASSERT(BytesAllocated + Size <= PoolSize);

        void* Result = nullptr;
        if (BytesAllocated + Size <= PoolSize)
        {
            Result = (u8*)PoolData + BytesAllocated;
            BytesAllocated += Size;
        }
        return Result;
    }
};

struct MemPoolTransient
{
    static constexpr size_t MaxBlocks = 2048;

    size_t PoolSize = 0;
    void* PoolData = nullptr;
    size_t BytesAllocated = 0;
    size_t NumBlocks = 0;
    MemBlock Blocks[MaxBlocks] = {};

    static constexpr bool bDebugPrint = true;
    static constexpr bool bDebugPrintList = true;

    void Insert(size_t NewIndex, MemBlock NewBlock)
    {
        ASSERT(NumBlocks < MaxBlocks && NewIndex <= NumBlocks);
        for (size_t ShiftUpIdx = NumBlocks; ShiftUpIdx > NewIndex; ShiftUpIdx--)
        {
            Blocks[ShiftUpIdx] = Blocks[ShiftUpIdx - 1];
        }
        Blocks[NewIndex] = NewBlock;
        NumBlocks++;
    }

    void Remove(size_t IndexToRemove)
    {
        ASSERT(Blocks && NumBlocks < MaxBlocks && IndexToRemove < NumBlocks);
        for (size_t ShiftDownIdx = IndexToRemove; (ShiftDownIdx + 1) < NumBlocks; ShiftDownIdx++)
        {
            Blocks[ShiftDownIdx] = Blocks[ShiftDownIdx + 1];
        }
        Blocks[NumBlocks - 1] = {};
        NumBlocks--;
    }

    void Init(size_t InSize)
    {
        PoolSize = InSize;
        PoolData = malloc(InSize);
        NumBlocks = 1;
        Blocks[0] = { false, InSize, PoolData };
    }

    void Term()
    {
    #if _DEBUG
        static bool bDebugPrintOnTerm = true;
        if (bDebugPrint || bDebugPrintOnTerm)
        {
            Outf("[memory][debug] Terminating...\n");
            DebugPrintList();
        }
    #endif // _DEBUG

        PoolSize = 0;
        free(PoolData);
        PoolData = nullptr;
        NumBlocks = 0;
        Blocks[0] = {};
    }

    void* Alloc(size_t Size)
    {
        ASSERT(NumBlocks < MaxBlocks);

        // NOTE: This forces blocks to be 16-bit aligned
    #define FORCE_16BIT_ALIGNMENT() (1)
    #if FORCE_16BIT_ALIGNMENT()
        Size = (Size + 0xF) & ~0xF;
    #endif FORCE_16BIT_ALIGNMENT()

        int NewAllocIdx = -1;
        for (size_t Idx = 0; Idx < NumBlocks; Idx++)
        {
            if (!Blocks[Idx].bAlloc &&
                Blocks[Idx].Size >= Size)
            {
                NewAllocIdx = (int)Idx;
                break;
            }
        }
        ASSERT(NewAllocIdx != -1);

        void* Result = nullptr;
        if (NewAllocIdx != -1)
        {
            if (Blocks[NewAllocIdx].Size == Size)
            {
                Blocks[NewAllocIdx].bAlloc = true;
                Result = Blocks[NewAllocIdx].Data;
            }
            else if (Blocks[NewAllocIdx].Size > Size)
            {
                MemBlock NewFree = { false, Blocks[NewAllocIdx].Size - Size, (u8*)Blocks[NewAllocIdx].Data + Size };
                Insert(NewAllocIdx + 1, NewFree);

                Blocks[NewAllocIdx].bAlloc = true;
                Blocks[NewAllocIdx].Size = Size;
                Result = Blocks[NewAllocIdx].Data;
            }
            else { ASSERT(false); }
            BytesAllocated += Size;
        }

    #if _DEBUG
        if (bDebugPrint)
        {
            Outf("[memory] Alloc'd Ptr [%d] 0x%X of size %llu (0x%X)\n", NewAllocIdx, Result, Size, Size);
            DebugPrintList();
        }
    #endif // _DEBUG

        ASSERT(Result);
        return Result;
    }

    void Free(void* Ptr)
    {
        ASSERT(Ptr);
        ASSERT(NumBlocks);

        int IndexToFree = -1;
        for (size_t Idx = 0; Idx < NumBlocks; Idx++)
        {
            if (Blocks[Idx].bAlloc && Blocks[Idx].Data == Ptr) { IndexToFree = (int)Idx; break; }
        }
        ASSERT(IndexToFree != -1);

        if (IndexToFree != -1)
        {
            size_t FreedSize = Blocks[IndexToFree].Size;
            void* FreedData = Blocks[IndexToFree].Data;
            UNUSED_VAR(FreedData);

            bool bBeforeFree = IndexToFree ? !Blocks[IndexToFree - 1].bAlloc : false;
            bool bAfterFree = IndexToFree < (NumBlocks - 1) ? !Blocks[IndexToFree + 1].bAlloc : false;
            if (bBeforeFree && bAfterFree)
            {
                Blocks[IndexToFree - 1].Size += Blocks[IndexToFree].Size + Blocks[IndexToFree + 1].Size;
                Remove(IndexToFree + 1);
                Remove(IndexToFree);
                // TODO: Make this only one Remove() call
            }
            else if (bBeforeFree)
            {
                Blocks[IndexToFree - 1].Size += Blocks[IndexToFree].Size;
                Remove(IndexToFree);
            }
            else if (bAfterFree)
            {
                Blocks[IndexToFree].Size += Blocks[IndexToFree + 1].Size;
                Remove(IndexToFree + 1);
                Blocks[IndexToFree].bAlloc = false;
            }
            else
            {
                Blocks[IndexToFree].bAlloc = false;
            }

            BytesAllocated -= FreedSize;
        #if _DEBUG
            if (bDebugPrint)
            {
                Outf("[memory] Free'd Ptr 0x%X of size %llu (0x%X)\n", FreedData, FreedSize, FreedSize);
                DebugPrintList();
            }
        #endif // _DEBUG
        }
    }

#if _DEBUG
    void DebugPrintList()
    {
        Outf("[memory][debug] NumBlocks: %llu\tTotalAllocatedBytes: %llu\n", NumBlocks, BytesAllocated);
        for (size_t Idx = 0; Idx < NumBlocks; Idx++)
        {
            void* DataOffset = (void*)((u8*)Blocks[Idx].Data - (u8*)PoolData);
            Outf("[%llu] %s -> 0x%X\tSize: %llu (0x%X)\n", Idx, Blocks[Idx].bAlloc ? "ALLOC" : "FREE", DataOffset, Blocks[Idx].Size, Blocks[Idx].Size);
        }
        Outf("\n");
    }
#endif // _DEBUG
};

struct MemImpl
{
    static constexpr size_t PersistentSize = sizeof(MemPoolTransient);
    static constexpr size_t TransientSize = 1024ULL * 1024ULL * 1024ULL;

    static MemPoolPersistent* Persistent;
    static MemPoolTransient* Transient;

    static void Init()
    {
        ASSERT(!Persistent && !Transient);

        Persistent = (MemPoolPersistent*)malloc(sizeof(MemPoolPersistent));
        *Persistent = {};
        Persistent->Init(PersistentSize);

        Transient = (MemPoolTransient*)Persistent->Alloc(sizeof(MemPoolTransient));
        *Transient = {};
        Transient->Init(TransientSize);
    }

    static void Term()
    {
        ASSERT(Persistent && Transient);

        Transient->Term();
        Transient = nullptr;

        Persistent->Term();
        free(Persistent);
        Persistent = nullptr;
    }

    static void* Alloc(size_t Size)
    {
        return Transient->Alloc(Size);
    }

    static void Free(void* Ptr)
    {
        ASSERT(Ptr);
        if (Ptr) { Transient->Free(Ptr); }
    }
};

MemPoolPersistent* MemImpl::Persistent = nullptr;
MemPoolTransient* MemImpl::Transient = nullptr;

void Memory::Init()
{
    MemImpl::Init();
}

void Memory::Term()
{
    MemImpl::Term();
}

void* Memory::Alloc(size_t Size)
{
    return MemImpl::Alloc(Size);
}

void Memory::Free(void* Ptr)
{
    MemImpl::Free(Ptr);
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

