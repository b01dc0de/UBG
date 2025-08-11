#ifndef ARRAY_H
#define ARRAY_H

template <typename T>
struct Array
{
    size_t Capacity;
    size_t Num;
    T* Data;

    static constexpr size_t DefaultInitCapacity = 32;
    static constexpr float DefaultGrowthFactor = 2.0f;

    Array(size_t InitCapacity)
    {
        Capacity = InitCapacity;
        Num = 0u;
        Data = new T[Capacity];
    }
    Array()
    {
        Capacity = DefaultInitCapacity;
        Num = 0u;
        Data = new T[Capacity];
    }
    Array& operator=(const Array& Other) = delete;
    Array(const Array& Other) = delete;
    Array& operator=(Array&& Other)
    {
        Capacity = Other.Capacity;
        Num = Other.Num;
        Data = Other.Data;

        Other.Capacity = 0u;
        Other.Num = 0u;
        Other.Data = nullptr;

        return *this;
    }
    Array(Array&& Other)
    {
        (void)operator=((Array&&)Other);
    }
    ~Array() = default;

    void Grow()
    {
        T* OldData = Data;

        Capacity = (size_t)(Capacity * DefaultGrowthFactor);
        Data = new T[Capacity];

        memcpy(Data, OldData, sizeof(T) * Num);
        delete[] OldData;
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

    /* NOTE: Preserves item order */
    void Remove(size_t RemovalIdx)
    {
        ASSERT(RemovalIdx < Num);
        for (size_t Idx = RemovalIdx; (Idx + 1) < Num; Idx++)
        {
            Data[Idx] = Data[Idx + 1];
        }
        Data[--Num] = {};
    }

    /* Note: Does not preserve order */
    void RemoveQ(size_t RemovalIdx)
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

