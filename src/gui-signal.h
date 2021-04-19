#pragma once

#include "core.h"

#include <map>
#include <vector>
#include <utility>
#include <functional>

namespace sig
{
	template<class T>
	using Slot = std::function<T>;

	using SlotID = u32;

	constexpr SlotID null_slot = std::numeric_limits<SlotID>::max();


	class Connection
	{
	private:
		using Release = void (*)(void*, SlotID);

		template<class T>
		friend class Signal;

		Connection(void* signal, Release func, SlotID id)
			: m_signal{signal}
			, m_release{func}
			, m_id{id}
		{}


	public:
		Connection() = default;

		void release()
		{
			if (m_signal != nullptr)
				m_release(m_signal, m_id);
		}

		bool empty()
		{
			return m_signal == nullptr;
		}


	private:
		void* m_signal{};		
		Release m_release{};
		SlotID m_id{null_slot};
	};

	class ScopedConnection
	{
	public:
		ScopedConnection(Connection conn) : m_conn(std::move(conn))
		{}

		ScopedConnection(const ScopedConnection&) = delete;

		~ScopedConnection()
		{
			m_conn.release();
		}

		ScopedConnection& operator = (const ScopedConnection&) = delete;

		ScopedConnection& operator = (Connection conn)
		{
			m_conn = std::move(conn);
			return *this;
		}


	public:
		void release()
		{
			m_conn.release();
		}

		bool empty()
		{
			return m_conn.empty();
		}


	private:
		Connection m_conn;
	};

	template<class>
	class Signal;

	template<class Ret, class ... Args>
	class Signal<Ret(Args...)>
	{
	private:
		static void release(void* signal, SlotID id)
		{
			static_cast<Signal*>(signal)->disconnect(id);
		}


	public:
		using SigSlot = Slot<Ret(Args...)>;

		Connection connect(SigSlot slot)
		{
			SlotID id = acquireID();
			m_slots.insert({id, std::move(slot)});
			return Connection{this, &release, id};
		}

		void emit(Args ... args)
		{
			for (auto& [id, slot] : m_slots)
				slot(args...);
		}

		void disconnectAll()
		{
			m_slots.clear();
			m_ids.clear();
			m_head = null_slot;
		}


	private:
		void disconnect(SlotID id)
		{
			if (!validID(id)) // if all were disconnected
				return;

			m_slots.erase(id);
			releaseID(id);
		}


	private:
		SlotID acquireID()
		{
			if (m_head == null_slot)
			{
				SlotID id = m_ids.size();
				m_ids.push_back(id);
				return id;
			}
			SlotID id = m_head;
			m_head = m_ids[id];
			m_ids[id] = id;
			return id;
		}

		void releaseID(SlotID id)
		{
			m_ids[id] = m_head;
			m_head = id;
		}

		bool validID(SlotID id)
		{
			return id < m_ids.size() && m_ids[id] == id;
		}


	private:
		std::map<SlotID, SigSlot> m_slots;

		std::vector<SlotID> m_ids;
		SlotID              m_head{null_slot};
	};
}
