#pragma once

#include "trb_util.h"

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


    struct default_compare_t
    {};

    constexpr default_compare_t default_compare{};

    struct default_allocator_t
    {};

    constexpr default_allocator_t default_allocator{};


    // simple example of usage of functions from trb namespace
    // to make it less vervose first Tree class must be created(iterator class is the same everywhere)
    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = DefaultAllocator>
    class Set
    {
    public:
        using Key = key_t;
        using Traits = trb::NodeTraits<Key, false>;
        using Node   = trb::TreeNode<Traits>;

        using Compare = compare_t;
        using Allocator = allocator_t<Node>;

        friend class Iterator;

        // almost corresponds to standart
        class Iterator
        {
            friend class Set;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = Key;
            using reference  = Key&;
            using pointer    = Key*;
            using difference_type = std::ptrdiff_t;


        private:
            Iterator(Node* nil, Node* node) : m_nil{nil}, m_node{node}
            {}

        public:
            Iterator()
            {}

        public:
            reference operator * () const
            {
                return m_node->key;
            }

            pointer operator -> () const
            {
                return &m_node->key;
            }

            Iterator& operator -- ()
            {
                m_node = trb::predecessor(m_nil, m_node);

                return *this;
            }

            Iterator& operator ++ ()
            {
                m_node = trb::successor(m_nil, m_node);

                return *this;
            }

            Iterator operator -- (int) const
            {
                auto it{*this};

                return --*this, it;
            }

            Iterator operator ++ (int) const
            {
                auto it{*this};

                return ++*this, it;
            }


            Iterator operator + (difference_type diff) const
            {
                auto it{*this};
                if (diff > 0) {
                    while (m_nil != m_node && diff > 0) {
                        ++it;
                        --diff;
                    }
                }
                else {
                    while (m_nil != m_node && diff < 0) {
                        --it;
                        ++diff;
                    }
                }
                return it;
            }

            Iterator operator - (difference_type diff) const
            {
                return *this + -diff;    
            }


            bool operator == (Iterator it) const
            {
                return m_node == it.m_node;
            }

            bool operator != (Iterator it) const
            {
                return m_node != it.m_node;
            }


        private:
            Node* m_nil {nullptr};
            Node* m_node{nullptr};
        };


    public:
        template<class Comp = Compare, class Alloc = Allocator>
        Set(Comp&& comp, Alloc&& alloc) 
            : m_compare(std::forward<Comp>(comp)), m_allocator(std::forward<Alloc>(alloc))
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator, std::enable_if_t<std::is_default_constructible_v<Alloc>, int> = 0>
        Set(Comp&& comp, default_allocator_t) 
            : m_compare(std::forward<Comp>(comp)), m_allocator()
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator, std::enable_if_t<std::is_default_constructible_v<Comp>, int> = 0>
        Set(Alloc&& alloc, default_compare_t)
            : m_compare(), m_allocator(std::forward<Alloc>(alloc))
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator, 
            std::enable_if_t<std::is_default_constructible_v<Comp> && std::is_default_constructible_v<Alloc>, int> = 0>
            Set() : m_compare(), m_allocator()
        {
            init();
        }

        // TODO : move & copy

        ~Set()
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

            m_root = trb::insert(m_nil, m_root, node, m_compare);
        }

        void erase(const Key& key)
        {
            if (auto node = trb::find(m_nil, m_root, m_compare, key); node != m_nil)
            {
                m_root = trb::remove(m_nil, m_root, node);

                m_allocator.dealloc(node);
            }
        }

        bool contains(const Key& key)
        {
            return find(key) != end();
        }


        Iterator find(const Key& key)
        {
            return Iterator{m_nil, trb::find(m_nil, m_root, m_compare, key)};
        }

        Iterator lowerBound(const Key& key)
        {
            return Iterator{m_nil, trb::lower_bound(m_nil, m_root, m_compare, key)};
        }

        Iterator upperBound(const Key& key)
        {
            return Iterator{m_nil, trb::upper_bound(m_nil, m_root, m_compare, key)};
        }

        Iterator begin()
        {
            return Iterator{m_nil, trb::tree_min(m_nil, m_root)};
        }

        Iterator end()
        {
            return Iterator{m_nil, m_nil};
        }


        void clear()
        {
            trb::clear(m_nil, m_root, [=] (Node* ptr) {m_allocator.dealloc(ptr);});
        }

        bool empty() const
        {
            return m_nil == m_root;
        }


    private:
        Node* m_nil{};
        Node* m_root{};

        Compare   m_compare;
        Allocator m_allocator;
    };

    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = DefaultAllocator>
    class ListSet
    {
    public:
        using Key = key_t;
        using Traits = trb::NodeTraits<Key, true>;
        using Node   = trb::TreeNode<Traits>;

        using Compare = compare_t;
        using Allocator = allocator_t<Node>;

        friend class Iterator;

        // almost corresponds to standart
        class Iterator
        {
            friend class ListSet;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = Key;
            using reference  = Key&;
            using pointer    = Key*;
            using difference_type = std::ptrdiff_t;


        private:
            Iterator(Node* nil, Node* node) : m_nil{nil}, m_node{node}
            {}

        public:
            Iterator()
            {}

        public:
            reference operator * () const
            {
                return m_node->key;
            }

            pointer operator -> () const
            {
                return &m_node->key;
            }

            Iterator& operator -- ()
            {
                m_node = trb::predecessor(m_nil, m_node);

                return *this;
            }

            Iterator& operator ++ ()
            {
                m_node = trb::successor(m_nil, m_node);

                return *this;
            }

            Iterator operator -- (int) const
            {
                auto it{*this};

                return --*this, it;
            }

            Iterator operator ++ (int) const
            {
                auto it{*this};

                return ++*this, it;
            }


            Iterator operator + (difference_type diff) const
            {
                auto it{*this};
                if (diff > 0) {
                    while (m_nil != m_node && diff > 0) {
                        ++it;
                        --diff;
                    }
                }
                else {
                    while (m_nil != m_node && diff < 0) {
                        --it;
                        ++diff;
                    }
                }
                return it;
            }

            Iterator operator - (difference_type diff) const
            {
                return *this + -diff;    
            }


            bool operator == (Iterator it) const
            {
                return m_node == it.m_node;
            }

            bool operator != (Iterator it) const
            {
                return m_node != it.m_node;
            }


        private:
            Node* m_nil {nullptr};
            Node* m_node{nullptr};
        };


    public:
        template<class Comp = Compare, class Alloc = Allocator>
        ListSet(Comp&& comp, Alloc&& alloc) 
            : m_compare(std::forward<Comp>(comp)), m_allocator(std::forward<Alloc>(alloc))
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator, std::enable_if_t<std::is_default_constructible_v<Alloc>, int> = 0>
        ListSet(Comp&& comp, default_allocator_t) 
            : m_compare(std::forward<Comp>(comp)), m_allocator()
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator, std::enable_if_t<std::is_default_constructible_v<Comp>, int> = 0>
        ListSet(Alloc&& alloc, default_compare_t)
            : m_compare(), m_allocator(std::forward<Alloc>(alloc))
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator, 
            std::enable_if_t<std::is_default_constructible_v<Comp> && std::is_default_constructible_v<Alloc>, int> = 0>
            ListSet() : m_compare(), m_allocator()
        {
            init();
        }

        // TODO : move & copy

        ~ListSet()
        {
            clear();

            deinit();
        }

    private:
        void init()
        {
            m_nil = m_allocator.alloc();
            m_nil->left  = m_nil;
            m_nil->right = m_nil;
            m_nil->next = m_nil;
            m_nil->prev = m_nil;
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

            m_root = trb::insert(m_nil, m_root, node, m_compare);
        }

        void erase(const Key& key)
        {
            if (auto node = trb::find(m_nil, m_root, m_compare, key); node != m_nil)
            {
                m_root = trb::remove(m_nil, m_root, node);

                m_allocator.dealloc(node);
            }
        }

        bool contains(const Key& key)
        {
            return find(key) != end();
        }


        Iterator find(const Key& key)
        {
            return Iterator{m_nil, trb::find(m_nil, m_root, m_compare, key)};
        }

        Iterator lowerBound(const Key& key)
        {
            return Iterator{m_nil, trb::lower_bound(m_nil, m_root, m_compare, key)};
        }

        Iterator upperBound(const Key& key)
        {
            return Iterator{m_nil, trb::upper_bound(m_nil, m_root, m_compare, key)};
        }

        Iterator begin()
        {
            return Iterator{m_nil, trb::tree_min(m_nil, m_root)};
        }

        Iterator end()
        {
            return Iterator{m_nil, m_nil};
        }


        void clear()
        {
            trb::clear(m_nil, m_root, [=] (Node* ptr) {m_allocator.dealloc(ptr);});
        }

        bool empty() const
        {
            return m_nil == m_root;
        }


    private:
        Node* m_nil{};
        Node* m_root{};

        Compare   m_compare;
        Allocator m_allocator;
    };
}