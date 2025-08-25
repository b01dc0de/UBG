#include "UBG.h" // Includes Memory.h

#define USE_NEW_MEMORY_MODEL() (1)

#if USE_NEW_MEMORY_MODEL()
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

    static constexpr bool bDebugPrint = false;
    static constexpr bool bDebugPrintList = false;

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
        static bool bDebugPrintOnTerm = true;
        if (bDebugPrint || bDebugPrintOnTerm)
        {
            Outf("[memory][debug] Terminating...\n");
            DebugPrintList();
        }

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

        if (bDebugPrint)
        {
            Outf("[memory] Alloc'd Ptr [%d] 0x%X of size %llu (0x%X)\n", NewAllocIdx, Result, Size, Size);
            DebugPrintList();
        }

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

#else // USE_NEW_MEMORY_MODEL()
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
    static constexpr size_t MaxBlocks = 2048;

    size_t DataSize;
    void* DataPool;
    MemBlock AllocBlocks[MaxBlocks];
    MemBlock FreeBlocks[MaxBlocks];
    size_t NumAlloc;
    size_t NumFree;
    size_t TotalAllocSize;
    size_t TotalFreeSize;

#if _DEBUG
    static constexpr bool bDebugPrint = true;
    size_t TotalNumAllocs;
    size_t TotalNumFrees;
    size_t MaxBytesUsed;
#endif // _DEBUG

    static void Insert(MemBlock* Blocks, size_t NumBlocks, size_t NewIndex, MemBlock NewBlock)
    {
        ASSERT(Blocks);
        ASSERT(NumBlocks < MaxBlocks);
        ASSERT(NewIndex < NumBlocks);

        for (size_t ShiftUpIdx = NumBlocks; ShiftUpIdx > NewIndex; ShiftUpIdx--)
        {
            Blocks[ShiftUpIdx] = Blocks[ShiftUpIdx - 1];
        }
        Blocks[NewIndex] = NewBlock;
    }

    static void Remove(MemBlock* Blocks, size_t NumBlocks, size_t IndexToRemove)
    {
        ASSERT(Blocks);
        ASSERT(NumBlocks < MaxBlocks);
        ASSERT(IndexToRemove < NumBlocks);

        for (size_t ShiftDownIdx = IndexToRemove; (ShiftDownIdx + 1) < NumBlocks; ShiftDownIdx++)
        {
            Blocks[ShiftDownIdx] = Blocks[ShiftDownIdx + 1];
        }
        Blocks[NumBlocks - 1] = {};
    }

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

        TotalAllocSize = 0;
        TotalFreeSize = Size;

    #if _DEBUG
        TotalNumAllocs = 0;
        TotalNumFrees = 0;
        MaxBytesUsed = 0;
    #endif // _DEBUG
    }

    void Term()
    {
        ASSERT(DataSize && DataPool);

    #if _DEBUG
        static bool bPrintOnTerm = true;
        if (bDebugPrint || bPrintOnTerm)
        {
            Outf("[memory][debug] Terminating...\n");
            Outf("\tNum allocs / frees: %llu / %llu\n", TotalNumAllocs, TotalNumFrees);
            double MaxBytesUsedPercent = (double)MaxBytesUsed / (double)DataSize * 100.0;
            Outf("\tMax bytes used: %llu (%0.2f%%)\n", MaxBytesUsed, MaxBytesUsedPercent);
            DebugPrint(true);
        }
    #endif // _DEBUG

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

        // NOTE: This forces blocks to be 16-bit aligned
        Size = (Size + 0xF) & ~0xF;

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
        bool bSplitFreeBlock = false;
        if (FoundIdx >= 0)
        {
            MemBlock NewAlloc = {};
            if (FreeBlocks[FoundIdx].Size == Size)
            {
                NewAlloc = FreeBlocks[FoundIdx];

                Remove(FreeBlocks, NumFree, FoundIdx);
            }
            else if (FreeBlocks[FoundIdx].Size > Size)
            {
                bSplitFreeBlock = true;
                NewAlloc = { Size, FreeBlocks[FoundIdx].Data };
                FreeBlocks[FoundIdx].Data = (u8*)FreeBlocks[FoundIdx].Data + Size;
                FreeBlocks[FoundIdx].Size -= Size;
            }
            ASSERT(NewAlloc.Data && NewAlloc.Size == Size);

            // Case 1: NewAlloc is the first alloc
            if (NumAlloc == 0)
            {
                AllocBlocks[0] = NewAlloc;
            }
            // Case 2: NewAlloc comes before the first alloc
            else if (NewAlloc.Data < AllocBlocks[0].Data)
            {
                Insert(AllocBlocks, NumAlloc, 0, NewAlloc);
            }
            // Case 3: NewAlloc comes after last alloc
            else if (NewAlloc.Data > AllocBlocks[NumAlloc - 1].Data)
            {
                AllocBlocks[NumAlloc] = NewAlloc;
            }
            // Case 4: NewAlloc comes in the middle of other allocs
            else
            {
                int NewAllocIdx = -1;
                for (int Idx = 0; (Idx + 1) < NumAlloc; Idx++)
                {
                    if (AllocBlocks[Idx].Data < NewAlloc.Data &&
                        NewAlloc.Data < AllocBlocks[Idx + 1].Data)
                    {
                        NewAllocIdx = Idx + 1;
                        break;
                    }
                }
                ASSERT(NewAllocIdx >= 0);
                if (NewAllocIdx >= 0)
                {
                    Insert(AllocBlocks, NumAlloc, NewAllocIdx, NewAlloc);
                }
            }

            TotalAllocSize += NewAlloc.Size;
            TotalFreeSize -= NewAlloc.Size;
            NumAlloc++;
            if (!bSplitFreeBlock)
            {
                NumFree--;
            }

        #if _DEBUG
            TotalNumAllocs++;
            if (TotalAllocSize > MaxBytesUsed)
            {
                MaxBytesUsed = TotalAllocSize;
            }
            if (bDebugPrint)
            {
                size_t PtrOffset = (u8*)NewAlloc.Data - (u8*)DataPool;
                Outf("[memory][debug] Alloc'd new ptr (%llu bytes) at 0x%X\n", NewAlloc.Size, PtrOffset);
                DebugPrint();
            }
        #endif // _DEBUG

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
            bool bCoalesced = false;
            bool bCoalescedWithBothAdjacent = false;
            int NewFreeIdx = -1;

            Remove(AllocBlocks, NumAlloc, FoundIdx);

            // Check if NewFree can be coalesced (is adjacent to other free blocks)
            void* NewFreeBegin = (u8*)NewFree.Data;
            void* NewFreeEnd = (u8*)NewFree.GetBlockEnd();

            // Case 1: No currently free blocks
            if (NumFree == 0)
            {
                NewFreeIdx = 0;
                FreeBlocks[NewFreeIdx] = NewFree;
            }
            // Case 2: NewFree will be coalesced with first block
            else if (NewFreeEnd == FreeBlocks[0].Data)
            {
                bCoalesced = true;
                FreeBlocks[0].Size += NewFree.Size;
                FreeBlocks[0].Data = (u8*)FreeBlocks[0].Data - NewFree.Size;
            }
            // Case 2: NewFree will be inserted at first
            else if (NewFreeEnd < FreeBlocks[0].Data)
            {
                NewFreeIdx = 0;
                Insert(FreeBlocks, NumFree, NewFreeIdx, NewFree);
            }
            // Case 3: NewFree will be coalesced with last block
            else if (NewFreeBegin == FreeBlocks[NumFree - 1].Data)
            {
                bCoalesced = true;
                FreeBlocks[NumFree - 1].Size += NewFree.Size;
            }
            // Case 4: NewFree will be appended to end
            else if (FreeBlocks[NumFree - 1].Data < NewFreeEnd)
            {
                NewFreeIdx = (int)NumFree;
                FreeBlocks[NewFreeIdx] = NewFree;
            }
            // Case 5: NewFree will be inserted/coalesced somewhere in the middle
            else
            {
                for (int Idx = 0; (Idx + 1) < NumFree; Idx++)
                {
                    u8* BeforeBlockEnd = (u8*)FreeBlocks[Idx].GetBlockEnd();
                    u8* NextBlockBegin = (Idx + 1) < NumFree ? (u8*)FreeBlocks[Idx + 1].Data : nullptr;

                    bool bFreeBefore = BeforeBlockEnd == NewFreeBegin;
                    bool bFreeAfter = NewFreeEnd == NextBlockBegin;

                    // Case 5.A: NewFree will be coalesced with blocks before and after
                    if (bFreeBefore && bFreeAfter)
                    {
                        size_t CoalescedSize = FreeBlocks[Idx].Size + NewFree.Size + FreeBlocks[Idx + 1].Size;
                        FreeBlocks[Idx].Size = CoalescedSize;

                        Remove(FreeBlocks, NumFree, Idx + 1);

                        bCoalesced = true;
                        bCoalescedWithBothAdjacent = true;
                        break;
                    }
                    // Case 5.B: NewFree will be coalesced with adjcent block before
                    else if (bFreeBefore)
                    {
                        size_t CoalescedSize = FreeBlocks[Idx].Size + NewFree.Size;
                        FreeBlocks[Idx].Size = CoalescedSize;

                        bCoalesced = true;
                        break;
                    }
                    // Case 5.C: NewFree will be coalesced with adjacent block after
                    else if (bFreeAfter)
                    {
                        size_t CoalescedSize = FreeBlocks[Idx + 1].Size + NewFree.Size;
                        FreeBlocks[Idx].Data = NewFree.Data;
                        FreeBlocks[Idx].Size = CoalescedSize;

                        bCoalesced = true;
                        break;
                    }

                    // Case 5.D: NewFree is between two alloc blocks
                    if (!bCoalesced &&
                        ((u8*)FreeBlocks[Idx].Data < (u8*)NewFree.Data) &&
                        ((u8*)NewFree.Data < (u8*)FreeBlocks[Idx + 1].Data))
                    {
                        NewFreeIdx = Idx + 1;
                        Insert(FreeBlocks, NumFree, NewFreeIdx, NewFree);
                        break;
                    }
                }
            }

            ASSERT(bCoalesced || NewFreeIdx >= 0);
            TotalAllocSize -= NewFree.Size;
            TotalFreeSize += NewFree.Size;
            NumAlloc--;
            if (!bCoalesced) { NumFree++; }
            else if (bCoalescedWithBothAdjacent) { NumFree--; }
        }
        else
        {
            ASSERT(false);
        }

    #if _DEBUG
        TotalNumFrees++;
        if (bDebugPrint)
        {
            size_t PtrOffset = (u8*)Ptr - (u8*)DataPool;
            Outf("[memory][debug] Free'd 0x%X\n", PtrOffset);
            DebugPrint();
        }
    #endif // _DEBUG
    }

#if _DEBUG
    void DebugPrint(bool bPrintList = false)
    {
        double PercentAlloc = ((double)TotalAllocSize / (double)DataSize) * 100.0;
        double PercentFree = ((double)TotalFreeSize / (double)DataSize) * 100.0;
        Outf("[memory][debug]\n\tTotal Allocated (%0.2f%%): %llu bytes\tNumBlocks: %llu\n", PercentAlloc, TotalAllocSize, NumAlloc);
        Outf("\tTotal Free (%0.2f%%): %llu bytes\tNumBlocks: %llu\n", PercentFree, TotalFreeSize, NumFree);
        size_t NextAllocIdx = 0;
        size_t NextFreeIdx = 0;
        size_t NumTotalBlocks = NumAlloc + NumFree;

        if (bPrintList)
        {
            for (size_t BlockIdx = 0; BlockIdx < NumTotalBlocks; BlockIdx++)
            {
                void* NextAlloc = NextAllocIdx < NumAlloc ? AllocBlocks[NextAllocIdx].Data : nullptr;
                void* NextFree = NextFreeIdx < NumFree ? FreeBlocks[NextFreeIdx].Data : nullptr;

                if (NextAlloc && NextFree)
                {
                    if (NextAlloc < NextFree)
                    {
                        size_t BlockOffset = (u8*)AllocBlocks[NextAllocIdx].Data - (u8*)DataPool;
                        Outf("\t Alloc[%d] : Size: %llu -> 0x%X\n", NextAllocIdx, AllocBlocks[NextAllocIdx].Size, BlockOffset);
                        NextAllocIdx++;
                    }
                    else if (NextFree < NextAlloc)
                    {
                        size_t BlockOffset = (u8*)FreeBlocks[NextFreeIdx].Data - (u8*)DataPool;
                        Outf("\t Free[%d] : Size: %llu -> 0x%X\n", NextFreeIdx, FreeBlocks[NextFreeIdx].Size, BlockOffset);
                        NextFreeIdx++;
                    }
                    else { ASSERT(false); }
                }
                else
                {
                    if (NextAlloc)
                    {
                        size_t BlockOffset = (u8*)AllocBlocks[NextAllocIdx].Data - (u8*)DataPool;
                        Outf("\t Alloc[%d] : Size: %llu -> 0x%X\n", NextAllocIdx, AllocBlocks[NextAllocIdx].Size, BlockOffset);
                        NextAllocIdx++;
                    }
                    else if (NextFree)
                    {
                        size_t BlockOffset = (u8*)FreeBlocks[NextFreeIdx].Data - (u8*)DataPool;
                        Outf("\t Free[%d] : Size: %llu -> 0x%X\n", NextFreeIdx, FreeBlocks[NextFreeIdx].Size, BlockOffset);
                        NextFreeIdx++;
                    }
                    else { ASSERT(false); }
                }
            }
        }
        Outf("\n");
    }
#endif // _DEBUG
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
#endif // USE_NEW_MEMORY_MODEL()

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

