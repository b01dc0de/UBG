#include "UBG.h" // Includes Memory.h

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
    static constexpr bool bDebugPrint = false;
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

