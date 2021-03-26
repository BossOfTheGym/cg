#pragma once

#include "core.h"
#include "trb_node.h"

#include <memory>
#include <cassert>

namespace trb
{
    template<class node_traits_t, class compare_t>
    struct TreeTraits
    {
        using Node    = TreeNode<node_traits_t>;
        using Key     = typename Node::Key;
        using Compare = compare_t;
    };

    template<class traits>
    class Tree
    {
        friend class Iterator;

    public:
        using Node    = typename traits::Node;
        using Key     = typename traits::Key;
        using Compare = typename traits::Compare;

        // almost corresponds to standart
        // TODO : decrementable end()
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
        template<class Comp = Compare, std::enable_if_t<std::is_default_constructible_v<Comp>, int> = 0>
        Tree() : m_compare()
        {}

        template<class Comp = Compare, std::enable_if_t<!std::is_default_constructible_v<Comp>, int> = 0>
        Tree(Comp&& compare = Comp()) : m_compare(std::forward<Comp>(compare))
        {}


    protected:
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
                if (node->right != m_nil)
                {
                    return min(node->right);
                }

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

        Node* lowerBound(Node* node, const Key& key)
        {
            Node* lb = m_nil;
            while (node != m_nil)
            {
                if (m_compare(node->key, key))
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

        Node* upperBound(Node* node, const Key& key)
        {
            Node* ub = m_nil;
            while (node != m_nil)
            {
                if (!m_compare(key, node->key))
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


        Node* searchInsert(Node* node, const Key& key)
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

        // TODO : needs to be tested, just another version of searchInsert
        Node* searchInsertUnique(Node* node, const Key& key)
        {
            Node* prev = m_nil;
            Node* curr = node;
            while (curr != m_nil)
            {
                prev = curr;
                if (m_compare(key, curr->key))
                    curr = curr->left;            
                else if (m_compare(curr->key, key))
                    curr = curr->right;
                else
                    break;
            }
            if (curr == m_nil) // key not found or root is nil, insert position found
                return prev;
            return searchInsert(curr, key); // search insert position
        }

        Node* find(Node* node, const Key& key)
        {
            Node* curr = node;
            while (curr != m_nil && (m_compare(curr->key, key) || m_compare(key, curr->key)))
            {
                if (m_compare(key, curr->key))
                    curr = curr->left;            
                else
                    curr = curr->right;
            }
            return curr;
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
        void insert(Node* node)
        {
            assert(node != m_nil);

            auto prev = searchInsert(m_root, node->key);

            node->parent = prev;
            if constexpr(Node::threaded)
            {
                if (prev == m_nil)
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
                    if (compare(node->key, prev->key))
                    {
                        prev->left = node;

                        // list insert
                        node->prev = prev->prev;
                        node->next = prev;
                        node->prev->next = node;

                        prev->prev = node;
                    }
                    else
                    {
                        prev->right = node;

                        // list insert
                        node->next = prev->next;
                        node->prev = prev;
                        node->next->prev = node;

                        prev->next = node;
                    }
                }
            }
            else // not threaded
            {
                if (prev == m_nil)
                {
                    m_root = node;
                }
                else
                {
                    if (m_compare(node->key, prev->key))
                        prev->left = node;
                    else
                        prev->right = node;
                }
            }
            node->left  = m_nil;
            node->right = m_nil;

            node->color = Color::Red;

            fixInsert(m_nil, root, node);
        }

        // NOTE : root != m_nil, after != m_nil, node != m_nil
        void insertAfter(Node* after, Node* node)
        {        
            assert(m_root != m_nil);
            assert(after != m_nil);
            assert(node != m_nil);

            if (after->right != m_nil)
            {
                Node* succ = successor(m_nil, after);

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

        // NOTE : root != m_nil, before != m_nil, node != m_nil
        void insertBefore(Node* before, Node* node)
        {
            assert(m_root != m_nil);
            assert(before != m_nil);
            assert(node != m_nil);

            if (before->left != m_nil)
            {
                Node* pred = predecessor(m_nil, before);

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


    protected:
        Node* m_nil{};
        Node* m_root{};

        Compare m_compare;
    };
}
