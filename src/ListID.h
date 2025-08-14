#ifndef LISTID_H
#define LISTID_H

using TypeID = u64;
static constexpr TypeID NumIDs = 32;

struct QueueID
{
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
            Available[Idx] = Idx;
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
        TypeID ID = Available[FirstIdx];
        FirstIdx = (FirstIdx + 1) % NumIDs;
        NumAvailable--;
        return ID;
    }

    void Push(TypeID ID)
    {
        ASSERT(NumAvailable < NumIDs);
        LastIdx = (LastIdx + 1) % NumIDs;
        Available[LastIdx] = ID;
        NumAvailable++;
    }
};

template <typename T>
struct ListID
{
    QueueID Queue;
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
        size_t NewIndex = NumActive;
        IDToIndexMap[ID] = NewIndex;
        IndexToIDMap[NewIndex] = ID;
        ActiveList[NewIndex] = NewItem;
        NumActive++;

        if (bDebugPrint)
        {
            Outf("[ListID][debug]: Created new ID %llu at index %llu\n", ID, NewIndex);
            DebugPrint();
        }

        return ID;
    }

    void Destroy(TypeID ID)
    {
        ASSERT(ID < NumIDs);

        // Copy element at end to deleted element to pack valid components:
        size_t IndexOfRemovedEntity = IDToIndexMap[ID];
        size_t IndexOfLastElement = NumActive - 1;
        ActiveList[IndexOfRemovedEntity] = ActiveList[IndexOfLastElement];

        // Update lookup maps:
        TypeID EntityOfLastElement = IndexToIDMap[IndexOfLastElement];
        IDToIndexMap[EntityOfLastElement] = IndexOfRemovedEntity;
        IndexToIDMap[IndexOfRemovedEntity] = EntityOfLastElement;

        Queue.Push(ID);
        NumActive--;

        if (bDebugPrint)
        {
            Outf("[ListID][debug]: Destroyed ID %llu at index %llu\n", ID, IndexOfRemovedEntity);
            DebugPrint();
        }
    }

    T* Get(TypeID ID)
    {
        ASSERT(IDToIndexMap[ID] < NumActive);
        T* Result = IDToIndexMap[ID] < NumActive ? &ActiveList[IDToIndexMap[ID]] : nullptr;
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

