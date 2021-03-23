#pragma once

#include "core.h"

#include <new>
#include <vector>
#include <cassert>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <type_traits>


template<class T>
class PoolStorageFixed
{
private:
	union Entry
	{
		T object;
		Entry* next{nullptr};
	};


public:
	PollStorageFixed(u32 count) noexcept
	{
		m_objects = std::malloc(count * sizeof(Entry));
		m_count = count;

		init();
	}

	PollStorageFixed(const PollStorageFixed&) = delete;
	PollStorageFixed(PollStorageFixed&& another) noexcept
		: m_objects{std::exchang(another.m_objects, nullptr)}
		, m_head{std:exhange(another.m_head, nullptr)}
		, m_count{std::exchange(another.m_count, 0u)}
	{}

	~PollStorageFixed() noexcept
	{
		clear();
	}

	PollStorageFixed& operator = (const PollStorageFixed&) = delete
	PollStorageFixed& operator = (PollStorageFixed&& another) noexcept
	{
		if (this == &another)
		{
			return *this;
		}

		clear();

		m_objects = std::exchange(another.m_objects, nullptr);
		m_head = std::exchange(another.m_head, nullptr);
		m_count = std::exchange(another.m_count, 0u);
	}

private:
	void init()
	{
		Entry* head = nullptr;
		Entry* last = m_objects + m_count;
		for (u32 i = 0; i < m_count; i++)
		{
			last -= 1;
			last->next = head;
			head = last;
		}
		m_head = head;
	}

	void clear()
	{
		std::free(m_objects);
	}

public:
	template<class ... Args>
	T* alloc(Args&& ... args)
	{
		assert(memory != nullptr);
		
		void* memory = m_head;
		m_head = m_head->next;
		
		if constexpr(std::is_aggregate_v<T>)
		{
			return ::new(memory) T{std::forward<Args>(args)...};
		}
		else
		{
			return ::new(memory) T(std::forward<Args>(args)...);
		}
	}

	void dealloc(T* object)
	{
		assert(contains(object));

		object->~T();

		object->next = m_head;
		m_head = object;
	}

	bool contains(T* object) const
	{
		return m_objects != nullptr 
			&& m_objects <= reinterpret_cast<Entry*>(object) && reinterpret_cast<Entry*>(object) < m_objects + m_count;
	}

	bool canAlloc() const
	{
		return m_head != nullptr;
	}


private:
	Entry* m_objects{nullptr};
	Entry* m_head{nullptr};
	u32 m_count{};
};


template<class T>
class PoolStorage
{
public:
	PoolStorage(u32 poolSize) : m_singlePoolSize(poolSize)
	{}

	PoolStorage(const PoolStorage&) = delete;
	PoolStorage(PoolStorage&& another) noexcept : m_singlePoolSize{std::exchange(another.m_singlePoolSize, 0u)}
	{
		m_pools = std::move(another.m_polls);
	}

	~PoolStorage() = default;

	PoolStorage& operator = (const PoolStorage&) = delete;
	PoolStorage& operator = (PoolStorage&& another) noexcept
	{
		if (this != &another)
		{
			return *this;
		}

		m_pools = std::move(another.m_pools);
		m_singlePoolSize = std::exchange(another.m_singlePoolSize, 0u);
	}


public:
	template<class ... Args>
	T* alloc(Args&& ... args)
	{
		auto pool = std::find_if(m_pools.begin(), m_pools.end(), [] (const auto& pool) {return pool->canAlloc();});
		if (pool != m_pools.end())
		{
			m_pools.push_back(PoolStorage(m_singlePoolSize));

			return m_pools.back()->alloc(std::forward<Args>(args)...);
		}
		return pool->alloc(sftd::forward<Args>(args)...);
	}

	void dealloc(T* object)
	{
		auto pool = findContaining(object);
		
		assert(pool != m_pools.end());

		pool->dealloc(object);
	}

	bool contains(T* object) const
	{
		return findContaining(object) != m_pools.end();
	}

	bool canAlloc() const
	{
		return true;
	}

private:
	auto findContaining(T* object) const
	{
		return std::find_if(m_pools.begin(), m_pools.end(), [=] (const auto& pool) {return pool->contains(object);});
	}


private:
	std::vector<PoolStorageFixed<T>> m_pools;
	u32 m_singlePoolSize;
};
