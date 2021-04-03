#pragma once

#include "trb_util.h"
#include "trb_tree.h"

#include <memory>
#include <utility>

namespace ds
{
    template<class T>
    class DefaultAllocator
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

    // tree_traits_t comes from trb_tree.h
    // node_traits_t comes from trb_node.h
     
    // TODO : noexcept correctness, I think I don't understand something
    // simple example of usage of functions from trb namespace
    // to make it less verbose, firstly, Tree class must be created(iterator class is the same everywhere)
    template<class tree_traits_t, template<class> class allocator_t = DefaultAllocator>
    class BasicSet : public trb::Tree<tree_traits_t>
    {
    public:
        using TreeTraits = tree_traits_t;
        using Tree       = trb::Tree<TreeTraits>;
        using Key        = typename TreeTraits::Key;
        using Node       = typename TreeTraits::Node;
        using Compare    = typename TreeTraits::Compare;
        using NodeTraits = typename TreeTraits::NodeTraits;

        using Allocator = allocator_t<Node>;

        using Iterator = typename Tree::Iterator;

        using Tree::m_nil;
        using Tree::m_root;

    public:
        BasicSet()
        {
            init();
        }

        BasicSet(const Allocator& alloc) : m_allocator(alloc)
        {
            init();
        }

        BasicSet(Allocator&& alloc) noexcept(std::is_nothrow_move_constructible_v<Allocator>)
            : m_allocator(std::move(alloc))
        {
            init();
        }

        BasicSet(const Compare& comp) : Tree(comp)
        {
            init();
        }

        BasicSet(Compare&& comp) : Tree(std::move(comp))
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator>
        BasicSet(Comp&& comp, Alloc&& alloc) noexcept(std::is_nothrow_constructible_v<Allocator, Alloc&&>)
            : Tree(std::forward<Comp>(comp))
            , m_allocator(std::forward<Alloc>(alloc))
        {
            init();
        }

        // TODO : move & copy

        ~BasicSet()
        {
            clear();

            deinit();
        }

    private:
        void init()
        {
            m_nil = m_allocator.alloc();
            m_nil->left   = m_nil;
            m_nil->right  = m_nil;
            m_nil->parent = m_nil;
            m_nil->color = trb::Color::Black;

            m_root = m_nil;
        }

        void deinit()
        {
            m_allocator.dealloc(m_nil);
        }


    public:
        void insert(const Key& key)
        {
            auto node = m_allocator.alloc(key);

            Tree::insert(node);
        }

        void erase(const Key& key)
        {
            if (auto node = Tree::find(m_root, key); node != m_nil)
            {
                Tree::remove(node);

                m_allocator.dealloc(node);
            }
        }

        bool contains(const Key& key)
        {
            return find(key) != end();
        }


        Iterator find(const Key& key)
        {
            return Tree::iter(Tree::find(m_root, key));
        }

        Iterator lowerBound(const Key& key)
        {
            return Tree::iter(Tree::lowerBound(m_root, key));
        }

        Iterator upperBound(const Key& key)
        {
            return Tree::iter(Tree::upperBound(m_root, key));
        }

        Iterator begin()
        {
            return Tree::iter(Tree::min(m_root));
        }

        Iterator end()
        {
            return Tree::iter();
        }


        void clear()
        {
            for (auto it = begin(), e = end(); it != e; ++it)
            {
                Tree::remove(it);

                m_allocator.dealloc(Tree::iterNode(it));
            }
        }

        bool empty() const
        {
            return m_nil == m_root;
        }


    private:
        Allocator m_allocator;
    };

    
    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = DefaultAllocator>
    using Set = BasicSet<trb::TreeTraits<key_t, compare_t, false>, allocator_t>;

    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = DefaultAllocator>
    using ListSet = BasicSet<trb::TreeTraits<key_t, compare_t, true>, allocator_t>;
}
