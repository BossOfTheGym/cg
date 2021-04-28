#pragma once

#include "core.h"
#include "primitive.h"

#include <memory>
#include <array>
#include <queue>
#include <vector>
#include <cassert>
#include <numeric>
#include <algorithm>
#include <type_traits>


namespace qtree
{
	using Vec2 = prim::Vec2;
	using AABB = prim::AABB2;

	enum class Leaf : u32
	{
		SW = 0b00,
		SE = 0b01,
		NW = 0b10,
		NE = 0b11,
	};

	namespace
	{
		AABB leaf_AABB(Leaf leaf, const AABB& aabb)
		{
			auto dv = (aabb.v1 - aabb.v0) * Vec2::value_type(0.5);

			auto v0 = aabb.v0;
			v0.x += dv.x * ((u32)leaf & 0x01);
			v0.y += dv.y * (((u32)leaf & 0x02) >> 1);

			return {v0, v0 + dv};
		}
	}

	template<class T>
	class Allocator
	{
	public:
		template<class ... Args>
		T* alloc(Args&& ... args)
		{
			if constexpr(std::is_aggregate_v<T>)
				return ::new T{std::forward<Args>(args)...};
			else
				return ::new T(std::forward<Args>(args)...);
		}

		void dealloc(T* ptr)
		{
			::delete ptr;
		}
	};

	template<class element_t>
	struct QuadNode
	{
		using Elem = element_t;

		bool empty() const
		{
			return data.empty();
		}

		bool leaf() const
		{
			return children[0] == nullptr;
		}


		// don't call if current node is a leaf
		bool allLeaves() const
		{
			return std::all_of(std::begin(children), std::end(children), [] (auto child) {return child->leaf();});
		}

		// don't call if current node is a leaf
		u32 leaves() const
		{
			return std::count_if(std::begin(children), std::end(children), [] (auto child) {return child->leaf();});
		}

		// don't call if current node is a leaf
		u32 emptyChildren() const
		{
			return std::count_if(std::begin(children), std::end(children), [] (auto child) {return child->empty();});
		}

		// don't call if current node is a leaf
		u32 nonEmptyChildren() const
		{
			return std::count_if(std::begin(children), std::end(children), [] (auto child) {return !child->empty();});
		}


		auto find(const Elem& elem)
		{
			return std::find(std::begin(data), std::end(data), elem);
		}

		bool has(const Elem& elem)
		{
			return find(elem) != std::end(data);
		}

		bool put(const Elem& elem)
		{
			if (!has(elem))
			{
				data.push_back(elem);
				return true;
			}
			return false;
		}

		bool rem(const Elem& elem)
		{
			if (auto it = find(elem); it != std::end(data))
			{
				std::swap(*it, data.back());
				data.pop_back();

				return true;
			}
			return false;
		}


		AABB box{};
		std::vector<Elem> data;
		std::array<QuadNode*, 4> children{nullptr};
	};

	// allocator_t has two methods:
	// 1) T* alloc(Args&& ... args) - constructs an object from args and returns pointer to it
	// 2) void dealloc(T* object) - deconstructs an object under the pointer
	// 
	// position_t is a functor mapping element_t to Vec2 (so you can store whatever you want here)
	// TODO : maybe I need to do deduction guides
	template<class element_t, class position_t, template<class T> class allocator_t = Allocator>
	class QuadTree
	{
	public:
		using Elem = element_t;
		using Node = QuadNode<element_t>;
		using Position = position_t;
		using NodeAllocator = allocator_t<Node>;


	public:
		explicit QuadTree(const AABB& box)
		{
			allocRoot(box);
		}

		template<class PositionT, class NodeAllocatorT>
		QuadTree(const AABB& box, PositionT&& position, NodeAllocatorT&& alloc) 
			noexcept(std::is_nothrow_constructible_v<Position, PositionT&&> && std::is_nothrow_constructible_v<NodeAllocator, NodeAllocatorT&&>)
			: m_position(std::forward<PositionT>(position))
			, m_allocator(std::forward<NodeAllocatorT>(alloc))
		{
			allocRoot(box);
		}

		// for now
		QuadTree(const QuadTree&) = delete;
		// for now
		QuadTree(QuadTree&&) noexcept = delete;

		~QuadTree()
		{
			clear();

			deallocRoot();
		}

		// for now
		QuadTree& operator = (const QuadTree&) = delete;
		// for now
		QuadTree& operator = (QuadTree&&) noexcept = delete;

	private:
		void allocRoot(const AABB& box)
		{
			m_root = m_allocator.alloc(box);
		}

		void deallocRoot()
		{
			m_allocator.dealloc(m_root);
		}


		bool contains(Node* node, const Elem& elem) const
		{
			return prim::inAABB(node->box, m_position(elem));
		}

		// NOTE : node is not empty, node != nullptr
		bool sameAs(Node* node, const Elem& elem) const
		{
			return m_position(node->data[0]) == m_position(elem);
		}

		// NOTE : node is not empty, node != nullptr
		bool allSame(Node* node) const
		{
			return std::all_of(std::begin(node->data), std::end(node->data),
				[&] (const auto& elem) {return m_position(elem) == m_position(node->data[0]);});
		}

		// NOTE : node is not empty, node != nullptr, all elements are same
		void subdivide(Node* node)
		{
			assert(node != nullptr);
			assert(!node->empty() && allSame(node));

			node->children[(u32)Leaf::SW] = m_allocator.alloc(leaf_AABB(Leaf::SW, node->box));
			node->children[(u32)Leaf::SE] = m_allocator.alloc(leaf_AABB(Leaf::SE, node->box));
			node->children[(u32)Leaf::NW] = m_allocator.alloc(leaf_AABB(Leaf::NW, node->box));
			node->children[(u32)Leaf::NE] = m_allocator.alloc(leaf_AABB(Leaf::NE, node->box));

			assert(node->children[(u32)Leaf::SW] != nullptr);
			assert(node->children[(u32)Leaf::SE] != nullptr);
			assert(node->children[(u32)Leaf::NW] != nullptr);
			assert(node->children[(u32)Leaf::NE] != nullptr);

			for (auto& child : node->children)
			{
				if (contains(child, node->data[0]))
				{
					child->data = std::move(node->data);
					break;
				}
			}
		}

		// NODE : node != nullptr, node is empty, node is not a leaf, node has only leaves 
		// and count of non-empty children is less than or equal to 1
		void unite(Node* node)
		{
			assert(node != nullptr);
			assert(node->empty());
			assert(!node->leaf());
			assert(node->allLeaves());
			assert(node->nonEmptyChildren() <= 1);

			for (auto& child : node->children)
			{
				while (!child->data.empty())
				{
					node->data.push_back(std::move(child->data.back()));

					child->data.pop_back();
				}
			}

			m_allocator.dealloc(node->children[(u32)Leaf::SW]); node->children[(u32)Leaf::SW] = nullptr;
			m_allocator.dealloc(node->children[(u32)Leaf::SE]); node->children[(u32)Leaf::SE] = nullptr;
			m_allocator.dealloc(node->children[(u32)Leaf::NW]); node->children[(u32)Leaf::NW] = nullptr;
			m_allocator.dealloc(node->children[(u32)Leaf::NE]); node->children[(u32)Leaf::NE] = nullptr;
		}


	private:
		bool insert(Node* node, const Elem& elem)
		{
			u32 depth = 0;
			while(depth < m_maxDepth)
			{
				if (node->leaf())
				{
					if (node->empty() || sameAs(node, elem))
						// node->empty(), inserts element -> true
						// node->same(), all elements in the node are the same, try to put
						//		true if elements wasn't present in the node
						//		false if was
						return node->put(elem);
					else
						// never called if max_depth was reached
						subdivide(node);
				}

				for (auto& child : node->children)
				{
					if (contains(child, elem))
					{
						node = child;

						break;
					}
				}

				++depth;
			}

			// max depth reached, node must be a leaf
			assert(node != nullptr);
			assert(node->leaf());

			// element could've already been added -> false
			// elemnts wasn't present in tree -> true
			return node->put(elem);
		}

		bool remove(Node* node, const Elem& elem)
		{
			std::vector<Node*> stack;			
			while (!node->leaf())
			{
				stack.push_back(node);
				for (auto& child : node->children)
				{
					if (contains(child, elem))
					{
						node = child;
						break;
					}
				}
			}

			if (!node->rem(elem))
				// no element was found
				return false;

			if (!node->empty() && !allSame(node))
				// node is not empty and elements are not same we cannot unite any nodes
				return true;

			// here we unite nodes
			// node->empty() or node->allSame() so the following is possible:
			// 1) node->empty() and node->allSame()
			// 2) !node->empty() and node->allSame()
			// 3) node->empty() and !node->allSame()
			// 1 <=> 3, it doesn't matter if elements are same or not if there are no elements
			while (!stack.empty())
			{
				auto prev = stack.back();
				stack.pop_back();

				if (!prev->allLeaves())
					break;

				if (prev->nonEmptyChildren() <= 1) // if all leaves are empty (so was node) or we ascend from all-same node
					unite(prev);
			}	
			return true;
		}

		void clear(Node* root)
		{
			// deallocates only children so root is untouched
			if (root != nullptr)
			{
				for (auto& child : root->children)
				{
					clear(child);

					m_allocator.dealloc(child);
				}
			}
		}

		void query(Node* node, const AABB& box, std::vector<Elem>& result)
		{
			if (!prim::overlaps(box, node->box))
				return;

			if (!node->leaf())
			{
				for (auto& child : node->children)
					query(child, box, result);
			}
			else
			{
				for (auto& elem : node->data)
				{
					if (prim::inAABB(box, m_position(elem)))
						result.push_back(elem);
				}
			}
		}

	public:
		bool insert(const Elem& elem)
		{
			return insert(m_root, elem);
		}

		bool remove(const Elem& elem)
		{
			return remove(m_root, elem);
		}

		void clear()
		{
			clear(m_root);
			for (auto& child : m_root->children)
				child = nullptr;
		}

		void query(const AABB& box, std::vector<Elem>& result)
		{
			result.clear();

			query(m_root, box, result);
			/*std::queue<Node*> nodes;

			nodes.push(m_root);
			while(!nodes.empty())
			{
				auto curr = nodes.front();
				nodes.pop();

				if (!prim::overlaps(box, curr->box))
					continue;

				if (!curr->leaf())
				{
					for (auto& child : curr->children)
						nodes.push(child);
				}
				else
				{
					for (auto& elem : curr->data)
					{
						if (prim::inAABB(box, m_position(elem)))
							result.push_back(elem);
					}
				}
			}*/
		}
		

	protected:
		Position m_position;
		NodeAllocator m_allocator;

		Node* m_root{nullptr};
		u32 m_maxDepth{16};
	};


	// helper to access dependent types from QuadTree
	template<class element_t, class position_t, template<class T> class allocator_t>
	struct Helper
	{
		using Tree = QuadTree<element_t, position_t, allocator_t>;
		using Elem = element_t;
		using Position = position_t;
		using NodeAllocator = allocator_t<typename Tree::Node>;
	};
}
