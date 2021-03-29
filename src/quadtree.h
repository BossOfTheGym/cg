#pragma once

#include "core.h"
#include "primitive.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <array>
#include <numeric>


// TODO : more effective insertion(has || put -> find, put)
// TODO : query
// TODO : capacity = 1u specialization
// TODO : get rid of stupid optimization from wikipedia, remove capacity parameter.
//		this implementation has a bug: it will infinitely subdivide if two equal point are given
namespace qtree
{
	using vec2 = prim::vec2;
	using AABB = prim::AABB2;

	enum class Leaf : u32
	{
		SW = 0b00,
		SE = 0b01,
		NW = 0b10,
		NE = 0b11,
	};

	AABB leaf_AABB(Leaf leaf, const AABB& aabb)
	{
		auto dv = (aabb.v1 - aabb.v0) * 0.5f;

		auto v0 = aabb.v0;
		v0.x += dv.x * ((u32)leaf & 0x01);
		v0.y += dv.y * (((u32)leaf & 0x02) >> 1);

		return {v0, v0 + dv};
	}


	template<u32 CAPACITY = 4u>
	struct QuadNode
	{
		bool contains(vec2* vec) const
		{
			return inAABB(box, *vec);
		}

		bool empty() const
		{
			return std::all_of(std::begin(data), std::end(data), [] (auto datum) {return datum == nullptr;});
		}

		bool leaf() const
		{
			return children[0] == nullptr;
		}

		bool allLeaves() const
		{
			return std::all_of(std::begin(children), std::end(children), [] (auto child) {return child->leaf();});
		}


		auto findNull()
		{
			return std::find_if(std::begin(data), std::end(data), [] (auto datum) {return datum == nullptr;});
		}

		auto findNonNull()
		{
			return std::find_if(std::begin(data), std::end(data), [] (auto datum) {return datum != nullptr;});
		}

		auto find(vec2* vec)
		{
			return std::find(std::begin(data), std::end(data), vec);
		}


		bool has(vec2* vec)
		{
			return find(vec) != std::end(data);
		}

		bool put(vec2* vec)
		{
			if (auto it = findNull(); it != std::end(data))
			{
				*it = vec;

				return true;
			}

			return false;
		}

		bool rem(vec2* vec)
		{
			if (auto it = find(vec); it != std::end(data))
			{
				*it = nullptr;

				return true;
			}

			return false;
		}


		AABB box{};

		std::array<vec2*, CAPACITY> data{nullptr};

		std::array<QuadNode*, 4> children{nullptr};
	};

	template<u32 CAPACITY_>
	struct QuadTraits
	{
		static constexpr u32 CAPACITY = CAPACITY_;

		using QuadNode = QuadNode<CAPACITY>;
	};


	template<class Traits>
	struct DefaultStorage
	{
		using QuadNode = typename Traits::QuadNode;

		QuadNode* alloc(const AABB& box)
		{
			return ::new QuadNode{box};
		}

		void dealloc(QuadNode* node)
		{
			delete node;
		}
	};

	template<u32 CAPACITY = 4U, template<class Traits> class Storage = DefaultStorage>
	class QuadTree : Storage<QuadTraits<CAPACITY>>
	{
		using QuadNode = typename QuadTraits<CAPACITY>::QuadNode;
		using StorageBase = Storage<QuadTraits<CAPACITY>>;


	public:
		template<class ... Args>
		QuadTree(const AABB& box, Args&& ... args) : StorageBase(std::forward<Args>(args)...)
		{
			m_root = StorageBase::alloc(box);
		}

		~QuadTree()
		{
			clear();

			StorageBase::dealloc(m_root); m_root = nullptr;
		}

	private:
		static u32 elems(QuadNode* node)
		{
			return std::count_if(std::begin(node->data), std::end(node->data), [] (auto datum) {return datum != nullptr;});
		}

		static u32 leafElems(QuadNode* node)
		{
			return std::accumulate(std::begin(node->children), std::end(node->children), 0u, 
				[] (auto val, auto child) {return val + elems(child);}
			);
		}

		template<class Ptr>
		static auto moveData(QuadNode* node, Ptr ptr)
		{
			auto newPtr = std::copy_if(std::begin(node->data), std::end(node->data), ptr, [] (auto datum) {return datum != nullptr;});
			std::fill(std::begin(node->data), std::end(node->data), nullptr);
			return newPtr;
		}

	private:
		void subdivide(QuadNode* node)
		{
			node->children[(u32)Leaf::SW] = StorageBase::alloc(leaf_AABB(Leaf::SW, node->box));
			node->children[(u32)Leaf::SE] = StorageBase::alloc(leaf_AABB(Leaf::SE, node->box));
			node->children[(u32)Leaf::NW] = StorageBase::alloc(leaf_AABB(Leaf::NW, node->box));
			node->children[(u32)Leaf::NE] = StorageBase::alloc(leaf_AABB(Leaf::NE, node->box));

			assert(node->children[(u32)Leaf::SW] != nullptr);
			assert(node->children[(u32)Leaf::SE] != nullptr);
			assert(node->children[(u32)Leaf::NW] != nullptr);
			assert(node->children[(u32)Leaf::NE] != nullptr);

			for (auto i = std::begin(node->data), e = std::end(node->data); i != e; i++)
			{
				auto inserted 
				 = node->children[(u32)Leaf::SW]->contains(*i) && node->children[(u32)Leaf::SW]->put(*i)
				|| node->children[(u32)Leaf::SE]->contains(*i) && node->children[(u32)Leaf::SE]->put(*i)
				|| node->children[(u32)Leaf::NW]->contains(*i) && node->children[(u32)Leaf::NW]->put(*i)
				|| node->children[(u32)Leaf::NE]->contains(*i) && node->children[(u32)Leaf::NE]->put(*i);

				*i = nullptr;

				assert(inserted);
			}
		}

		void unite(QuadNode* node)
		{
			auto ptr = std::begin(node->data);
			ptr = moveData(node->children[(u32)Leaf::SW], ptr);
			ptr = moveData(node->children[(u32)Leaf::SE], ptr);
			ptr = moveData(node->children[(u32)Leaf::NW], ptr);
			ptr = moveData(node->children[(u32)Leaf::NE], ptr);

			StorageBase::dealloc(node->children[(u32)Leaf::SW]); node->children[(u32)Leaf::SW] = nullptr;
			StorageBase::dealloc(node->children[(u32)Leaf::SE]); node->children[(u32)Leaf::SE] = nullptr;
			StorageBase::dealloc(node->children[(u32)Leaf::NW]); node->children[(u32)Leaf::NW] = nullptr;
			StorageBase::dealloc(node->children[(u32)Leaf::NE]); node->children[(u32)Leaf::NE] = nullptr;
		}

	private:
		bool insert(QuadNode* root, vec2* vec)
		{
			if (!root->contains(vec))
				return false;

			if (root->leaf())
			{
				if (root->has(vec) || root->put(vec))
					return true;

				subdivide(root);
			}

			auto inserted = insert(root->children[(u32)Leaf::SW], vec)
				|| insert(root->children[(u32)Leaf::SE], vec)
				|| insert(root->children[(u32)Leaf::NW], vec)
				|| insert(root->children[(u32)Leaf::NE], vec);

			assert(inserted);

			return inserted;
		}

		bool remove(QuadNode* root, vec2* vec)
		{
			if (!root->contains(vec))
				return false;

			if (root->leaf())
				return root->has(vec) && root->rem(vec);

			auto removed = remove(root->children[(u32)Leaf::SW], vec)
				|| remove(root->children[(u32)Leaf::SE], vec)
				|| remove(root->children[(u32)Leaf::NW], vec)
				|| remove(root->children[(u32)Leaf::NE], vec);

			if (root->allLeaves() && leafElems(root) <= CAPACITY / 2) // can be differrent strategy here
				unite(root);

			return removed;
		}

		void clear(QuadNode* root)
		{
			if (root != nullptr)
			{
				for (auto& child : root->children)
				{
					clear(child);

					StorageBase::dealloc(child); child = nullptr;
				}
			}
		}

	public:
		bool insert(vec2* vec)
		{
			return insert(m_root, vec);
		}

		bool remove(vec2* vec)
		{
			return remove(m_root, vec);
		}

		void clear()
		{
			clear(m_root);
		}


	private:
		QuadNode* m_root{nullptr};
	};
}
