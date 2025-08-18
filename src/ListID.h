#ifndef LISTID_H
#define LISTID_H

// TODO: Implement a generational index as well to verify
//       A given current ID value is still the one expected

using TypeID = u64;
static constexpr TypeID DefaultNumIDs = 1024;

template <TypeID NumIDs = DefaultNumIDs>
struct QueueID
{
    static constexpr TypeID MinValidID = 1;
    static constexpr TypeID MaxValidID = NumIDs;
    static_assert(MaxValidID > MinValidID);
    SArray<TypeID, NumIDs> Available;
    u64 NumAvailable;
    u64 FirstIdx;
    u64 LastIdx;

    void Init()
    {
        Available = {};
        Available.Init();
        for (size_t Idx = 0u; Idx < NumIDs; Idx++)
        {
            Available[Idx] = Idx + 1;
        }
        NumAvailable = NumIDs;
        FirstIdx = 0;
        LastIdx = NumIDs - 1;
    }

    void Term()
    {
        Available.Term();
    }

    TypeID Pop()
    {
        ASSERT(NumAvailable);
        TypeID ID = 0;
        if (NumAvailable)
        {
            ID = Available[FirstIdx];
            FirstIdx = (FirstIdx + 1) % NumIDs;
            NumAvailable--;
        }
        return ID;
    }

    void Push(TypeID ID)
    {
        ASSERT(NumAvailable < NumIDs);
        LastIdx = (LastIdx + 1) % NumIDs;
        Available[LastIdx] = ID;
        NumAvailable++;
    }

    bool IsValid(TypeID ID)
    {
        return MinValidID <= ID && ID <= MaxValidID;
    }
};

template <typename T, TypeID NumIDs = DefaultNumIDs>
struct ListID
{
    QueueID<NumIDs> Queue;
    SArray<T, NumIDs> ActiveList;
    SArray<size_t, NumIDs> IDToIndexMap;
    SArray<TypeID, NumIDs> IndexToIDMap;
    size_t NumActive;

    static constexpr bool bDebugPrint = false;

    void Init()
    {
        Queue.Init();
        ActiveList.Init();
        IDToIndexMap.Init();
        IndexToIDMap.Init();
        NumActive = 0;
    }

    void Term()
    {
        Queue.Term();
        ActiveList.Term();
        IDToIndexMap.Term();
        IndexToIDMap.Term();
    }

    TypeID Create()
    {
        return Create(T{});
    }

    TypeID Create(T NewItem)
    {
        ASSERT(NumActive < NumIDs);
        TypeID ID = Queue.Pop();
        ASSERT(Queue.IsValid(ID));

        if (Queue.IsValid(ID))
        {
            TypeID RealID = ID - 1;
            size_t NewIndex = NumActive;
            IDToIndexMap[RealID] = NewIndex;
            IndexToIDMap[NewIndex] = RealID;
            ActiveList[NewIndex] = NewItem;
            NumActive++;

            if (bDebugPrint)
            {
                Outf("[ListID][debug]: Created new ID %llu at index %llu\n", RealID, NewIndex);
                DebugPrint();
            }
        }
        else
        {
            if (bDebugPrint)
            {
                Outf("[ListID][debug]: Failed to create new ID.\n", ID);
            }
        }

        return ID;
    }

    void Destroy(TypeID ID)
    {
        ASSERT(ID && Queue.IsValid(ID));
        ASSERT(NumActive);

        if (ID)
        {
            TypeID RealID = ID - 1;

            // Copy element at end to deleted element to pack valid components:
            size_t IndexOfRemovedEntity = IDToIndexMap[RealID];
            size_t IndexOfLastElement = NumActive - 1;
            ActiveList[IndexOfRemovedEntity] = ActiveList[IndexOfLastElement];

            // Update lookup maps:
            TypeID EntityOfLastElement = IndexToIDMap[IndexOfLastElement];
            IDToIndexMap[EntityOfLastElement] = IndexOfRemovedEntity;
            IndexToIDMap[IndexOfRemovedEntity] = EntityOfLastElement;

            Queue.Push(RealID);
            NumActive--;

            if (bDebugPrint)
            {
                Outf("[ListID][debug]: Destroyed ID %llu at index %llu\n", ID, IndexOfRemovedEntity);
                DebugPrint();
            }
        }
    }

    T* Get(TypeID ID)
    {
        ASSERT(ID && Queue.IsValid(ID));
        T* Result = nullptr;
        if (ID)
        {
            TypeID RealID = ID - 1;
            ASSERT(IDToIndexMap[RealID] < NumActive);
            if (IDToIndexMap[RealID] < NumActive)
            {
                Result = &ActiveList[IDToIndexMap[RealID]];
            }
        }
        return Result;
    }

    void DebugPrint()
    {
        Outf("[ListID][debug]:\n");
        Outf("\tID Queue:\n");
        Outf("\t\tNumAvailable: %llu\n", Queue.NumAvailable);
        Outf("\t\tFirstIdx: [%llu] ->\t%llu\n", Queue.FirstIdx, Queue.Available[Queue.FirstIdx]);
        Outf("\t\tLastIdx: [%llu] ->\t%llu\n", Queue.LastIdx, Queue.Available[Queue.LastIdx]);
        Outf("\tNumActive: %llu\n", NumActive);
        constexpr bool bPrintMaps = true;
        if (NumActive && bPrintMaps)
        {
            Outf("\tIDToIndexMap:\n");
            for (size_t Idx = 0; Idx < NumActive; Idx++)
            {
                Outf("\t[%llu] -> %llu\n", Idx, IDToIndexMap[Idx]);
            }
            Outf("\n\tIndexToIDMap:\n");
            for (size_t Idx = 0; Idx < NumActive; Idx++)
            {
                Outf("\t[%llu] -> %llu\n", Idx, IndexToIDMap[Idx]);
            }
        }
    }
};

#endif // LISTID_H

