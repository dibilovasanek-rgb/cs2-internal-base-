#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
template <class T, class N = int>
class CUtlMemory
{
	enum
	{
		EXTERNAL_BUFFER_MARKER = -1,
		EXTERNAL_CONST_BUFFER_MARKER = -2,
	};

public:
	class Iterator_t
	{
	public:
		Iterator_t(const N nIndex) :
			nIndex(nIndex) {
		}

		bool operator==(const Iterator_t it) const
		{
			return nIndex == it.nIndex;
		}

		bool operator!=(const Iterator_t it) const
		{
			return nIndex != it.nIndex;
		}

		N nIndex;
	};

	CUtlMemory(const int nInitialGrowSize, const int nAllocationCount) :
		pMemory(nullptr), nAllocationCount(nAllocationCount), nGrowSize(nInitialGrowSize)
	{


	}

	CUtlMemory(T* pMemory, const int nElements) :
		pMemory(pMemory), nAllocationCount(nElements), nGrowSize(EXTERNAL_BUFFER_MARKER) {
	}

	CUtlMemory(const T* pMemory, const int nElements) :
		pMemory(pMemory), nAllocationCount(nElements), nGrowSize(EXTERNAL_CONST_BUFFER_MARKER) {
	}

	~CUtlMemory()
	{
	}

	CUtlMemory(const CUtlMemory&) = delete;

	CUtlMemory(CUtlMemory&& moveFrom) noexcept :
		pMemory(moveFrom.pMemory), nAllocationCount(moveFrom.nAllocationCount), nGrowSize(moveFrom.nGrowSize)
	{
		moveFrom.pMemory = nullptr;
		moveFrom.nAllocationCount = 0;
		moveFrom.nGrowSize = 0;
	}


	CUtlMemory& operator=(const CUtlMemory&) = delete;

	CUtlMemory& operator=(CUtlMemory&& moveFrom) noexcept
	{
		// copy member variables to locals before purge to handle self-assignment
		T* pMemoryTemp = moveFrom.pMemory;
		const int nAllocationCountTemp = moveFrom.nAllocationCount;
		const int nGrowSizeTemp = moveFrom.nGrowSize;

		moveFrom.pMemory = nullptr;
		moveFrom.nAllocationCount = 0;
		moveFrom.nGrowSize = 0;

		// if this is a self-assignment, Purge() is a no-op here

		pMemory = pMemoryTemp;
		nAllocationCount = nAllocationCountTemp;
		nGrowSize = nGrowSizeTemp;
		return *this;
	}

	[[nodiscard]] T& operator[](const N nIndex)
	{
		return pMemory[nIndex];
	}

	[[nodiscard]] const T& operator[](const N nIndex) const
	{
		return pMemory[nIndex];
	}

	[[nodiscard]] T& Element(const N nIndex)
	{
		return pMemory[nIndex];
	}

	[[nodiscard]] const T& Element(const N nIndex) const
	{
		return pMemory[nIndex];
	}

	[[nodiscard]] T* Base()
	{
		return pMemory;
	}

	[[nodiscard]] const T* Base() const
	{
		return pMemory;
	}

	[[nodiscard]] int AllocationCount() const
	{
		return nAllocationCount;
	}

	[[nodiscard]] bool IsExternallyAllocated() const
	{
		return nGrowSize <= EXTERNAL_BUFFER_MARKER;
	}

	[[nodiscard]] static N InvalidIndex()
	{
		return static_cast<N>(-1);
	}

	[[nodiscard]] bool IsValidIndex(N nIndex) const
	{
		return (nIndex >= 0) && (nIndex < nAllocationCount);
	}

	[[nodiscard]] Iterator_t First() const
	{
		return Iterator_t(IsValidIndex(0) ? 0 : InvalidIndex());
	}

	[[nodiscard]] Iterator_t Next(const Iterator_t& it) const
	{
		return Iterator_t(IsValidIndex(it.nIndex + 1) ? it.nIndex + 1 : InvalidIndex());
	}

	[[nodiscard]] N GetIndex(const Iterator_t& it) const
	{
		return it.nIndex;
	}

	[[nodiscard]] bool IsIndexAfter(N nIndex, const Iterator_t& it) const
	{
		return nIndex > it.nIndex;
	}

	[[nodiscard]] bool IsValidIterator(const Iterator_t& it) const
	{
		return IsValidIndex(it.index);
	}

	[[nodiscard]] Iterator_t InvalidIterator() const
	{
		return Iterator_t(InvalidIndex());
	}



protected:
	T* pMemory; // 0x00
	int nAllocationCount; // 0x04
	int nGrowSize; // 0x08
};

template <class T, int nAlignment>
class CUtlMemoryAligned : public CUtlMemory<T>
{
public:
	// @note: not implemented
	//CS_CLASS_NO_INITIALIZER(CUtlMemoryAligned);
};

template <class T, std::size_t SIZE, class I = int>
class CUtlMemoryFixedGrowable : public CUtlMemory<T, I>
{
	typedef CUtlMemory<T, I> BaseClass;

public:
	CUtlMemoryFixedGrowable(int nInitialGrowSize = 0, int nInitialSize = SIZE) :
		BaseClass(arrFixedMemory, SIZE)
	{
		nMallocGrowSize = nInitialGrowSize;
	}

	void Grow(int nCount = 1)
	{
		if (this->IsExternallyAllocated())
			this->ConvertToGrowableMemory(nMallocGrowSize);

		BaseClass::Grow(nCount);
	}

	void EnsureCapacity(int nCapacity)
	{
		if (CUtlMemory<T>::nAllocationCount >= nCapacity)
			return;

		if (this->IsExternallyAllocated())
			// can't grow a buffer whose memory was externally allocated
			this->ConvertToGrowableMemory(nMallocGrowSize);

		BaseClass::EnsureCapacity(nCapacity);
	}

private:
	int nMallocGrowSize;
	T arrFixedMemory[SIZE];
};

template <typename T, std::size_t SIZE, int nAlignment = 0>
class CUtlMemoryFixed
{
public:
	CUtlMemoryFixed(const int nGrowSize = 0, const int nInitialCapacity = 0)
	{
	}

	CUtlMemoryFixed(const T* pMemory, const int nElements)
	{
	}

	[[nodiscard]] static constexpr bool IsValidIndex(const int nIndex)
	{
		return (nIndex >= 0) && (nIndex < SIZE);
	}

	// specify the invalid ('null') index that we'll only return on failure
	static constexpr int INVALID_INDEX = -1;

	[[nodiscard]] static constexpr int InvalidIndex()
	{
		return INVALID_INDEX;
	}

	[[nodiscard]] T* Base()
	{
		if (nAlignment == 0)
			return reinterpret_cast<T*>(&pMemory[0]);

		return reinterpret_cast<T*>((reinterpret_cast<std::uintptr_t>(&pMemory[0]) + nAlignment - 1) & ~(nAlignment - 1));
	}

	[[nodiscard]] const T* Base() const
	{
		if (nAlignment == 0)
			return reinterpret_cast<T*>(&pMemory[0]);

		return reinterpret_cast<T*>((reinterpret_cast<std::uintptr_t>(&pMemory[0]) + nAlignment - 1) & ~(nAlignment - 1));
	}

	[[nodiscard]] T& operator[](int nIndex)
	{
		return Base()[nIndex];
	}

	[[nodiscard]] const T& operator[](int nIndex) const
	{
		return Base()[nIndex];
	}

	[[nodiscard]] T& Element(int nIndex)
	{
		return Base()[nIndex];
	}

	[[nodiscard]] const T& Element(int nIndex) const
	{
		return Base()[nIndex];
	}

	[[nodiscard]] int AllocationCount() const
	{
		return SIZE;
	}

	[[nodiscard]] int Count() const
	{
		return SIZE;
	}

	void Grow(int nCount = 1)
	{
	}

	void EnsureCapacity(const int nCapacity)
	{
	}

	void Purge() {}

	void Purge(const int nElements)
	{
	}

	[[nodiscard]] bool IsExternallyAllocated() const
	{
		return false;
	}

	class Iterator_t
	{
	public:
		Iterator_t(const int nIndex) :
			nIndex(nIndex) {
		}

		bool operator==(const Iterator_t it) const
		{
			return nIndex == it.nIndex;
		}

		bool operator!=(const Iterator_t it) const
		{
			return nIndex != it.nIndex;
		}

		int nIndex;
	};

	[[nodiscard]] Iterator_t First() const
	{
		return Iterator_t(IsValidIndex(0) ? 0 : InvalidIndex());
	}

	[[nodiscard]] Iterator_t Next(const Iterator_t& it) const
	{
		return Iterator_t(IsValidIndex(it.nIndex + 1) ? it.nIndex + 1 : InvalidIndex());
	}

	[[nodiscard]] int GetIndex(const Iterator_t& it) const
	{
		return it.nIndex;
	}

	[[nodiscard]] bool IsIndexAfter(int i, const Iterator_t& it) const
	{
		return i > it.nIndex;
	}

	[[nodiscard]] bool IsValidIterator(const Iterator_t& it) const
	{
		return IsValidIndex(it.nIndex);
	}

	[[nodiscard]] Iterator_t InvalidIterator() const
	{
		return Iterator_t(InvalidIndex());
	}

private:
	char pMemory[SIZE * sizeof(T) + nAlignment];
};

template <typename T>
class c_utl_vector {
public:
	c_utl_vector()
	{
		m_size = 0;
		m_elements = nullptr;
	}

	T& element(int i);
	const T& element(int i) const;

	T& operator[](int i)
	{
		return m_elements[i];
	}

	int count() const
	{
		return m_size;
	}

	int m_size;
	T* m_elements;

	auto begin() const { return m_elements; }
	auto end() const { return m_elements + m_size; }

	auto at(int i) const { return m_elements[i]; }
	auto at_ptr(int i) const { return m_elements + i; }

	bool exists(T val) const {
		for (const auto& it : *this)
			if (it == val) return true;
		return false;
	}
	bool is_empty() const { return m_size == 0; }

	int find(const T& source) const
	{
		for (int i = 0; i < m_size; ++i)
		{
			if (m_elements[i] == source)
				return i;
		}

		return -1;
	}
};

template <typename K, typename V>
class c_utl_map {
public:
    struct node_t { int m_left, m_right, m_parent; std::uint8_t m_tag; K m_key; V m_value; };
    std::byte comparator[8]{};
    int m_allocation_count{};
    uint32_t m_allocation_flags{};
    node_t* m_data{};
    int m_root{-1}, m_size{}, m_first_free{-1}, m_last_alloc{-1};
    bool is_valid() const {
        return m_allocation_count>=0 && m_allocation_count<=1048576 && m_size>=0 && m_size<=m_allocation_count &&
            (m_size==0 || (m_data && (m_allocation_flags&0x7fffffff) && m_root>=0 && m_root<m_allocation_count));
    }
    bool valid_node(int index) const {
        return m_data && index>=0 && index<m_allocation_count && m_data[index].m_parent!=index && (m_data[index].m_tag==0 || m_data[index].m_tag==1);
    }
    int first(int index) const {
        for (int steps=0;steps<m_allocation_count && valid_node(index);++steps) {
            const int left=m_data[index].m_left;
            if (left==-1) return index;
            index=left;
        }
        return -1;
    }
    int next(int index) const {
        if (!valid_node(index)) return -1;
        if (m_data[index].m_right!=-1) return first(m_data[index].m_right);
        for (int steps=0;steps<m_allocation_count;++steps) {
            const int parent=m_data[index].m_parent;
            if (!valid_node(parent)) return -1;
            if (m_data[parent].m_left==index) return parent;
            if (m_data[parent].m_right!=index) return -1;
            index=parent;
        }
        return -1;
    }
    struct iterator {
        const c_utl_map* map; int index; int visited;
        const node_t& operator*() const { return map->m_data[index]; }
        iterator& operator++() { index=++visited<map->m_size ? map->next(index) : -1; return *this; }
        bool operator!=(const iterator& other) const { return index!=other.index; }
    };
    iterator begin() const { return {this,is_valid() && m_size ? first(m_root) : -1,0}; }
    iterator end() const { return {this,-1,0}; }
    std::optional<V> find_by_key(K key) const {
        if (!is_valid()) return {};
        int current=m_root;
        for (int steps=0;steps<m_allocation_count && valid_node(current);++steps) {
            const auto& element=m_data[current];
            if (element.m_key<key) current=element.m_right;
            else if (element.m_key>key) current=element.m_left;
            else return element.m_value;
        }
        return {};
    }
};
static_assert(sizeof(c_utl_map<int, void*>)==0x28);
using FrlsMapLayout = c_utl_map<int, void*>;
static_assert(offsetof(FrlsMapLayout, m_data)==0x10);

template <typename T>
class c_network_utl_vector {
public:
	unsigned int m_size;
	T* m_elements;
};