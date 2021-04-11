#pragma once

#include "core.h"
#include "trb_node.h"

#include <memory>
#include <cassert>
#include <type_traits>

namespace trb
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

    // TODO : const iterator    
    // TODO : copy
    
    template<class key_t, class compare_t, template<class> class allocator_t, bool threaded_v, bool multi_v>
    struct TreeTraits
    {
        static constexpr const bool threaded = threaded_v;
        static constexpr const bool multi = multi_v;

        using NodeTraits = NodeTraits<key_t, threaded_v>;
        using Node       = TreeNode<NodeTraits>;
        using Key        = key_t;
        using Compare    = compare_t;
        using Allocator  = allocator_t<Node>;
    };

    template<class tree_traits_t>
    class Tree
    {
        friend class Iterator;

    public:
        using Node      = typename tree_traits_t::Node;
        using Key       = typename tree_traits_t::Key;
        using Compare   = typename tree_traits_t::Compare;
        using Allocator = typename tree_traits_t::Allocator;

        // almost corresponds to standart (even closer)
        class Iterator
        {
            friend class Tree;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = Key;
            using reference  = Key&;
            using pointer    = Key*;
            using difference_type = std::ptrdiff_t;


        private:
            Iterator(Tree* tree, Node* node) : m_tree{tree}, m_node{node}
            {}

        public:
            Iterator() = default;


        public:
            reference operator * () const
            {
                assert(m_node != m_tree->m_nil);

                return m_node->key;
            }

            pointer operator -> () const
            {
                assert(m_node != m_tree->m_nil);

                return &m_node->key;
            }

            Iterator& operator -- ()
            {
                m_node = m_tree->predecessor(m_node);

                return *this;
            }

            Iterator& operator ++ ()
            {
                m_node = m_tree->successor(m_node);

                return *this;
            }

            Iterator operator -- (int)
            {
                auto it{*this};

                return --(*this), it;
            }

            Iterator operator ++ (int)
            {
                auto it{*this};

                return ++(*this), it;
            }


            Iterator operator + (difference_type diff) const
            {
                auto it{*this};
                if (diff > 0)
                {
                    while (m_tree->m_nil != m_node && diff > 0) 
                    {
                        ++it;
                        --diff;
                    }
                }
                else 
                {
                    while (m_tree->m_nil != m_node && diff < 0)
                    {
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
            Tree* m_tree{nullptr};
            Node* m_node{nullptr};
        };


    public:
        Tree()
        {
            init();
        }

        Tree(Allocator&& alloc) : m_allocator(std::move(alloc))
        {
            init();
        }

        Tree(const Allocator& alloc) : m_allocator(alloc)
        {
            init();
        }

        Tree(Compare&& compare) : m_compare(std::move(compare))
        {
            init();
        }

        Tree(const Compare& compare) : m_compare(compare)
        {
            init();
        }

        template<class Comp = Compare, class Alloc = Allocator>
        Tree(Comp&& comp, Alloc&& alloc) 
            : m_compare(std::forward<Comp>(comp))
            , m_allocator(std::forward<Alloc>(alloc))
        {
            init();
        }


        Tree(Tree&& another) noexcept(std::is_nothrow_move_constructible_v<Compare> && std::is_nothrow_move_constructible_v<Allocator>)
            : m_root(std::exchange(another.m_root, nullptr))
            , m_nil(std::exchange(another.m_nil, nullptr))
            , m_compare(std::move(another.m_compare))
            , m_allocator(std::move(another.m_allocator))
        {}

        // NOTE : simplification, see TODO
        Tree(const Tree&) = delete;

        ~Tree()
        {
            deinit();
        }


        Tree& operator = (Tree&& another) noexcept(std::is_nothrow_move_assignable_v<Compare> && std::is_nothrow_move_assignable_v<Allocator>)
        {
            if (this != &another)
            {
                deinit();

                m_root = std::exchange(another.m_root, nullptr);
                m_nil = std::exchange(another.m_nil, nullptr);

                m_compare   = std::move(another.m_compare);
                m_allocator = std::move(another.m_allocator);
            }
            return *this;
        }

        // NOTE : simplification, see TODO
        Tree& operator = (const Tree&) = delete;

    private: // m_root & m_nil init
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
            clear();

            m_allocator.dealloc(m_nil);
        }


    protected: // all tree functions
        Node* min(Node* node)
        {
            while (node->left != m_nil)
                node = node->left;
            return node;
        }   
        
        Node* max(Node* node)
        {
            while (node->right != m_nil)
                node = node->right;
            return node;
        }

        Node* predecessor(Node* node)
        {
            if constexpr(!Node::threaded)
            {
                if (node == m_nil)
                    return max(m_root);

                if (node->left != m_nil)
                    return max(node->left);

                Node* pred = node->parent;
                while (pred != m_nil && node == pred->left)
                {
                    node = pred;
                    pred = pred->parent;
                }

                return pred;
            }
            else
            {
                return node->prev;
            }
        }

        Node* successor(Node* node)
        {
            if constexpr(!Node::threaded)
            {
                if (node == m_nil)
                    return min(m_root);

                if (node->right != m_nil)
                    return min(node->right);

                Node* succ = node->parent;
                while (succ != m_nil && node == succ->right)
                {
                    node = succ;
                    succ = succ->parent;
                }

                return succ;
            }
            else
            {
                return node->next;
            }
        }

        template<class key_t>
        Node* lowerBound(Node* node, key_t&& key)
        {
            Node* lb = m_nil;
            while (node != m_nil)
            {
                if (m_compare(node->key, key)) // node->key < key
                {
                    node = node->right;
                }
                else
                {
                    lb = node;
                    node = node->left;
                }
            }
            return lb;
        }

        template<class key_t>
        Node* upperBound(Node* node, key_t&& key)
        {
            Node* ub = m_nil;
            while (node != m_nil)
            {
                if (!m_compare(key, node->key)) // node->key <= key
                {
                    node = node->right;
                }
                else
                {
                    ub = node;
                    node = node->left;
                }
            }
            return ub;
        }

        u32 blackHeight(Node* node)
        {
            u32 count = 0;
            while (node != m_nil)
            {
                if (node->color == Color::Black)
                    ++count;
                node = node->left;
            }
            return count;
        }

        // NOTE : node != m_nil, node->right != m_nil
        void rotateLeft(Node* node)
        {
            assert(node != m_nil);
            assert(node->right != m_nil);

            auto pivot = node->right;

            node->right = pivot->left;

            pivot->left = node;
            if (node->right != m_nil)
                node->right->parent = node;

            if (node->parent != m_nil)
            {
                if (node == node->parent->left)
                    node->parent->left = pivot;            
                else
                    node->parent->right = pivot;
            }
            else
            {
                m_root = pivot;
            }

            pivot->parent = node->parent;
            node->parent = pivot;
        }

        // NOTE : node != m_nil, node->left != nullptr
        void rotateRight(Node* node)
        {
            assert(node != m_nil);
            assert(node->left != m_nil);

            auto pivot = node->left;

            node->left = pivot->right;
            pivot->right = node;
            if (node->left != m_nil)
                node->left->parent = node;

            if (node->parent != m_nil)
            {
                if (node->parent->left == node)
                    node->parent->left = pivot;
                else
                    node->parent->right = pivot;
            }
            else
            {
                m_root = pivot;
            }

            pivot->parent = node->parent;
            node->parent = pivot;
        }


        // NOTE : looks like upper-bound search which is suitable for multiset
        // each elemeent will be inserted after all equal
        template<class key_t>
        Node* searchInsertUB(Node* node, key_t&& key)
        {
            Node* prev = m_nil;
            Node* curr = node;
            while (curr != m_nil)
            {
                prev = curr;
                if (m_compare(key, curr->key))
                    curr = curr->left;
                else 
                    curr = curr->right;
            }
            return prev;
        }

        // NOTE : for set-like data structures (no duplicates)        
        struct SearchResultLB
        {
            Node* lb{};
            Node* prev{};
        };

        template<class key_t>
        SearchResultLB searchInsertLB(Node* node, key_t&& key)
        {
            Node* prev = m_nil;
            Node* lb   = m_nil;
            while (node != m_nil)
            {
                prev = node;
                if (m_compare(node->key, key)) // node->key < key
                {
                    node = node->right;
                }
                else
                {
                    lb = node;
                    node = node->left;
                }
            }
            return {lb, prev};
        }

        // NOTE : set case: returns insertPos and flag
        // if flag is false then element was found, returned node has this element(not multiset case)
        // for multiset flag is always true? element is always inserted
        // returned node can be nil (tree is empty)
        template<class key_t>
        std::pair<Node*, bool> searchInsert(Node* node, key_t&& key)
        {
            if constexpr(tree_traits_t::multi) // multiset
            {
                return {searchInsertUB(node, std::forward<key_t>(key)), true};
            }
            else // not multiset
            {
                auto [lb, lbPrev] = searchInsertLB(node, std::forward<key_t>(key));
                if (lb != m_nil && !m_compare(key, lb->key))
                    // lb->key >= node->key -> already true
                    // !(node->key < lb->key) <=> lb->key <= node->key -> if true then equal node was found
                    return {lb, false};
                // prev is insert position
                return {lbPrev, true};
            }
        }

        // NOTE : set case: returns node or nil if key wasn't found
        // multiset case: returns first node with equal key or nil if key wasn't found
        template<class key_t>
        Node* find(Node* node, key_t&& key)
        {
            if constexpr(!tree_traits_t::multi) // not multiset
            {
                Node* curr = node;
                while (curr != m_nil)
                {
                    if (m_compare(key, curr->key))
                        curr = curr->left;            
                    else if (m_compare(curr->key, key))
                        curr = curr->right;
                    else
                        break;
                }
                return curr;   
            }
            else // multiset
            {
                Node* lb = lowerBound(node, key);
                if (lb != m_nil && !m_compare(key, lb->key))
                    // lb->key >= key -> already true
                    // !(key < lb->key) <=> lb->key <= key -> if true then equal node was found
                    return lb;
                // node wasn't found
                return m_nil;
            }
        }


        void fixInsert(Node* node)
        {
            while (node->parent->color == Color::Red)
            {
                if (node->parent == node->parent->parent->left)
                {
                    auto uncle = node->parent->parent->right;
                    if (uncle->color == Color::Red)
                    {
                        node->parent->color = Color::Black;
                        uncle->color = Color::Black;
                        node->parent->parent->color = Color::Red;

                        node = node->parent->parent;
                    }
                    else
                    {
                        if (node == node->parent->right)
                        {
                            node = node->parent;

                            rotateLeft(node);
                        }

                        node->parent->color = Color::Black;
                        node->parent->parent->color = Color::Red;

                        rotateRight(node->parent->parent);
                    }
                }
                else
                {
                    auto uncle = node->parent->parent->left;
                    if (uncle->color == Color::Red)
                    {
                        node->parent->color = Color::Black;
                        uncle->color = Color::Black;
                        node->parent->parent->color = Color::Red;

                        node = node->parent->parent;
                    }
                    else
                    {
                        if (node == node->parent->left)
                        {
                            node = node->parent;

                            rotateRight(node);
                        }

                        node->parent->color = Color::Black;
                        node->parent->parent->color = Color::Red;

                        rotateLeft(node->parent->parent);
                    }
                }
            }

            m_root->color = Color::Black;
        }

        // NOTE : node != m_nil, node in default state except key        
        void insert(Node* insertPos, Node* node)
        {
            assert(insertPos != nullptr);
            assert(node != nullptr);
            assert(node != m_nil);

            node->parent = insertPos;
            if constexpr(Node::threaded)
            {
                if (insertPos == m_nil)
                {
                    m_root = node;

                    // list insert
                    node->prev = m_nil;
                    node->next = m_nil;

                    m_nil->next = node;
                    m_nil->prev = node;
                }
                else
                {
                    if (m_compare(node->key, insertPos->key))
                    {
                        insertPos->left = node;

                        // list insert
                        node->prev = insertPos->prev;
                        node->next = insertPos;
                        node->prev->next = node;

                        insertPos->prev = node;
                    }
                    else
                    {
                        insertPos->right = node;

                        // list insert
                        node->next = insertPos->next;
                        node->prev = insertPos;
                        node->next->prev = node;

                        insertPos->next = node;
                    }
                }
            }
            else // not threaded
            {
                if (insertPos == m_nil)
                {
                    m_root = node;
                }
                else
                {
                    if (m_compare(node->key, insertPos->key))
                        insertPos->left = node;
                    else
                        insertPos->right = node;
                }
            }
            // NOTE : can be ommited for more efficiency
            node->left  = m_nil;
            node->right = m_nil;

            node->color = Color::Red;

            fixInsert(node);
        }

        // NOTE : node != m_nil, m_root != m_nil, after != m_nil
        // NOTE : method makes no assumption on the order of the elements: it is your responsability to maintain it
        void insertAfter(Node* after, Node* node)
        {
            assert(m_root != m_nil);
            assert(after != m_nil);
            assert(node != m_nil);

            if (after->right != m_nil)
            {
                Node* succ = successor(after);

                succ->left = node;
                node->parent = succ;
            }
            else
            {
                after->right = node;
                node->parent = after;
            }
            node->color = Color::Red;
            node->left  = m_nil;
            node->right = m_nil;

            if constexpr(Node::threaded)
            {
                node->next = after->next;
                node->next->prev = node;
                node->prev = after;
                after->next = node;
            }

            fixInsert(node);
        }

        // NOTE : node != m_nil, m_root != m_nil, before != m_nil 
        // NOTE : method makes no assumption on the order of the elements: it is your responsability to maintain it
        void insertBefore(Node* before, Node* node)
        {       
            assert(m_root != m_nil);
            assert(before != m_nil);
            assert(node != m_nil);

            if (before->left != m_nil)
            {
                Node* pred = predecessor(before);

                pred->right= node;
                node->parent = pred;
            }
            else
            {
                before->left= node;
                node->parent = before;
            }
            node->color = Color::Red;
            node->left  = m_nil;
            node->right = m_nil;

            if constexpr(Node::threaded)
            {
                node->prev = before->prev;
                node->prev->next = node;
                node->next = before;
                before->prev = node;
            }

            fixInsert(node);
        }


        void transplant(Node* node, Node* trans)
        {
            if (node->parent != m_nil)
            {
                if (node == node->parent->left)
                    node->parent->left = trans;
                else
                    node->parent->right = trans;
            }
            else
            {
                m_root = trans;
            }
            trans->parent = node->parent;
        }

        void fixRemove(Node* restore)
        {
            while (restore != m_root && restore->color == Color::Black)
            {
                if (restore == restore->parent->left)
                {
                    auto brother = restore->parent->right;
                    if (brother->color == Color::Red)
                    {
                        brother->color = Color::Black;
                        restore->parent->color = Color::Red;

                        rotateLeft(restore->parent);

                        brother = restore->parent->right;
                    }

                    if (brother->left->color == Color::Black && brother->right->color == Color::Black)
                    {
                        brother->color = Color::Red;

                        restore = restore->parent;
                    }
                    else
                    {
                        if (brother->right->color == Color::Black)
                        {
                            brother->left->color = Color::Black;
                            brother->color = Color::Red;

                            rotateRight(brother);

                            brother = restore->parent->right;
                        }

                        brother->color = restore->parent->color;
                        restore->parent->color = Color::Black;
                        brother->right->color = Color::Black;

                        rotateLeft(restore->parent);

                        restore = m_root;
                    }
                }
                else
                {
                    auto brother = restore->parent->left;
                    if (brother->color == Color::Red)
                    {
                        brother->color = Color::Black;
                        restore->parent->color = Color::Red;

                        rotateRight(restore->parent);

                        brother = restore->parent->left;
                    }

                    if (brother->left->color == Color::Black && brother->right->color == Color::Black)
                    {
                        brother->color = Color::Red;

                        restore = restore->parent;
                    }
                    else
                    {
                        if (brother->left->color == Color::Black)
                        {
                            brother->right->color = Color::Black;
                            brother->color = Color::Red;

                            rotateLeft(brother);

                            brother = restore->parent->left;
                        }

                        brother->color = restore->parent->color;
                        restore->parent->color = Color::Black;
                        brother->left->color = Color::Black;

                        rotateRight(restore->parent);

                        restore = m_root;
                    }
                }
            }
            restore->color = Color::Black;
        }

        void remove(Node* node)
        {
            assert(node != m_nil);

            Color removed = node->color;
            Node* restore = m_nil;

            if (node->left == m_nil)
            {
                restore = node->right;

                transplant(node, node->right);
            }
            else if (node->right == m_nil)
            {
                restore = node->left;

                transplant( node, node->left);
            }
            else
            {
                Node* leftmost;
                if constexpr(Node::threaded)
                    leftmost = node->next;
                else
                    leftmost = min(node->right);

                removed = leftmost->color;
                restore = leftmost->right;
                if (restore == m_nil)
                    restore->parent = leftmost;

                if (node->right != leftmost)
                {
                    transplant(leftmost, leftmost->right);

                    leftmost->right = node->right;
                    node->right->parent = leftmost;
                }

                leftmost->left = node->left;
                node->left->parent = leftmost;

                transplant(node, leftmost);

                leftmost->color = node->color;
            }

            if constexpr(Node::threaded)
            {
                // list remove
                node->prev->next = node->next;
                node->next->prev = node->prev;
            }

            if (removed == Color::Black)
                fixRemove(restore);
            m_nil->parent = m_nil;
        }


    public: // methods returning iterators
        template<class key_t>
        std::pair<Iterator, bool> insert(key_t&& key)
        {
            auto [insertPos, canInsert] = searchInsert(m_root, std::forward<key_t>(key));
            if (canInsert)
            {
                Node* node = m_allocator.alloc(std::forward<key_t>(key));
                insert(insertPos, node);
                return {Iterator{this, node}, true};
            }
            return {Iterator{this, insertPos}, false};
        }

        template<class key_t>
        Iterator insertAfter(Iterator it, key_t&& key)
        {
            Node* node = m_allocator.alloc(std::forward<key_t>(key));

            if (empty())
            {
                insert(m_nil, node);

                return Iterator{this, node};
            }

            if (it == end())
            {
                insertBefore(min(m_root), node);

                return Iterator{this, node};
            }

            insertAfter(it.m_node, node);

            return Iterator{this, node};
        }

        template<class key_t>
        Iterator insertBefore(Iterator it, key_t&& key)
        {
            Node* node = m_allocator.alloc(std::forward<key_t>(key));

            if (empty())
            {
                insert(m_nil, node);

                return Iterator{this, node};
            }

            if (it == end())
            {
                insertAfter(max(m_root), node);

                return Iterator{this, node};
            }

            insertBefore(it.m_node, node);
            return Iterator{this, node};
        }

        template<class key_t>
        void erase(key_t&& key)
        {
            if (auto node = find(m_root, key); node != m_nil)
            {
                remove(node);

                m_allocator.dealloc(node);
            }
        }

        void erase(Iterator it)
        {
            assert(it != end());

            remove(it.m_node);

            m_allocator.dealloc(it.m_node);
        }

        template<class key_t>
        bool contains(key_t&& key)
        {
            return find(m_root, std::forward<key_t>(key)) != m_nil;
        }

        void clear()
        {
            Node* curr = min(m_root);
            while (curr != m_nil)
            {
                Node* next = successor(curr);
                remove(curr);
                m_allocator.dealloc(curr);
                curr = next;
            }
        }

        bool empty() const
        {
            return m_root == m_nil;
        }

        template<class key_t>
        Iterator find(key_t&& key)
        {
            return Iterator{this, find(m_root, std::forward<key_t>(key))};
        }
        
        template<class key_t>
        Iterator lowerBound(key_t&& key)
        {
            return Iterator{this, lowerBound(m_root, std::forward<key_t>(key))};
        }

        template<class key_t>
        Iterator upperBound(key_t&& key)
        {
            return Iterator{this, upperBound(m_root, std::forward<key_t>(key))};
        }

        Iterator begin()
        {
            return Iterator{this, min(m_root)};
        }

        Iterator end()
        {
            return Iterator{this, m_nil};
        }


    protected: // utilities for inheriting classes
        Iterator iter(Node* node)
        {
            return Iterator{this, node};
        }

        Iterator iter()
        {
            return Iterator{this, m_nil};
        }

        Node* iterNode(Iterator it)
        {
            return it.m_node;
        }


    protected:
        Node* m_nil{};
        Node* m_root{};

        Compare   m_compare;
        Allocator m_allocator;
    };
}
