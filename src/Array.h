#ifndef ARRAY_H
#define ARRAY_H

#include <string.h> // Needed for memcpy

/*
    NOTE(Chris):
        - SArray is like a traditional c-array, no knowledge of it's current 'Num', fixed-size
        - DArray is a dynamic array, supports Add/Remove and can expand as needed
*/

template <typename T, size_t Capacity>
struct SArray
{
    T* Data;

    void Init()
    {
        ASSERT(!Data);
        Data = new T[Capacity];
    }

    void Term()
    {
        ASSERT(Data);
        delete[] Data;
    }

    inline size_t Size()
    {
        return Capacity;
    }

    T* operator*()
    {
        return Data;
    }

    T& operator[](size_t Idx)
    {
        ASSERT(Data);
        ASSERT(Idx < Capacity);
        return Data[Idx];
    }
};

template <typename T>
struct DArray
{
    size_t Capacity;
    size_t Num;
    T* Data;

    static constexpr size_t DefaultInitCapacity = 32;
    static constexpr float DefaultGrowthFactor = 2.0f;

    void Init(size_t InitCapacity = DefaultInitCapacity)
    {
        Capacity = InitCapacity;
        Num = 0u;
        Data = new T[Capacity];
    }
    void Term()
    {
        if (Data)
        {
            delete[] Data;
            Capacity = 0;
            Num = 0;
            Data = nullptr;
        }
    }

    DArray(size_t InitCapacity = DefaultInitCapacity)
    {
        Init(InitCapacity);
    }
    DArray& operator=(const DArray& Other) = delete;
    DArray(const DArray& Other) = delete;
    DArray& operator=(DArray&& Other)
    {
        Capacity = Other.Capacity;
        Num = Other.Num;
        Data = Other.Data;

        Other.Capacity = 0u;
        Other.Num = 0u;
        Other.Data = nullptr;

        return *this;
    }
    DArray(DArray&& Other)
    {
        (void)operator=((DArray&&)Other);
    }
    ~DArray()
    {
        Term();
    }

    void Resize(size_t NewCapacity)
    {
        ASSERT(NewCapacity > Capacity); // Only support growing arrays for now
        if (Data)
        {
            T* OldData = Data;

            Capacity = NewCapacity;
            Data = new T[Capacity];

            memcpy(Data, OldData, sizeof(T) * Num);
            delete[] OldData;
        }
        else
        {
            Init(NewCapacity);
        }
    }
    void Reserve(size_t NewCapacity)
    {
        Resize(NewCapacity);
    }
    void Grow()
    {
        Resize((size_t)(Capacity * DefaultGrowthFactor));
    }

    void Add(const T& Item)
    {
        if (Num >= Capacity)
        {
            Grow();
        }

        Data[Num++] = Item;
    }
    void Add(const T* List, size_t ListNum)
    {
        size_t NewNum = Num + ListNum;
        while (NewNum >= Capacity)
        {
            Grow();
        }

        for (size_t Idx = Num; Idx < NewNum; Idx++)
        {
            Data[Idx] = List[Idx];
        }
        Num = NewNum;
    }
    void AddUninit(size_t Count)
    {
        size_t NewNum = Num + Count;
        while (NewNum >= Capacity)
        {
            Grow();
        }
        Num = NewNum;
    }

    /* NOTE: Preserves item order */
    //void Remove(size_t RemovalIdx)
    //{
    //    ASSERT(RemovalIdx < Num);
    //    for (size_t Idx = RemovalIdx; (Idx + 1) < Num; Idx++)
    //    {
    //        Data[Idx] = Data[Idx + 1];
    //    }
    //    Data[--Num] = {};
    //}

    /* Note: Does not preserve order */
    void Remove(size_t RemovalIdx)
    {
        ASSERT(RemovalIdx <= Num);
        if (Num)
        {
            Data[RemovalIdx] = Data[Num - 1];
            Data[Num - 1] = {};
        }
        else
        {
            Data[0] = {};
        }
        Num--;
    }

    T& operator[](size_t Idx)
    {
        ASSERT(Idx < Num);
        return Data[Idx];
    }
};

#endif // ARRAY_H

