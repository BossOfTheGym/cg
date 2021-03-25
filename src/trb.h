#pragma once

#include "core.h"

#include <iostream>
#include <memory>
#include <utility>

// TODO : create simple template
// TODO : iterator
// TODO : join, split
struct TreeRB
{
    enum class Color : char
    {
        Black = 0,
        Red = 1,
    };

    using Key = u32;


    static TreeRB* tree_min(TreeRB* nil, TreeRB* root)
    {
        while (root->left != nil)
        {
            root = root->left;
        }

        return root;
    }

    static TreeRB* tree_max(TreeRB* nil, TreeRB* root)
    {
        while (root->right != nil)
        {
            root = root->right;
        }

        return root;
    }

    static TreeRB* predecessor(TreeRB* nil, TreeRB* node)
    {
        if (node->left != nil)
        {
            return tree_max(nil, node->left);
        }

        TreeRB* pred = node->parent;
        while (pred != nil && node == pred->left)
        {
            node = pred;
            pred = pred->parent;
        }

        return pred;
    }

    static TreeRB* successor(TreeRB* nil, TreeRB* node)
    {
        if (node->right != nil)
        {
            return tree_min(nil, node->right);
        }

        TreeRB* succ = node->parent;
        while (succ != nil && node == succ->right)
        {
            node = succ;
            succ = succ->parent;
        }

        return succ;
    }

    static TreeRB* lower_bound(TreeRB* nil, TreeRB* root, const Key& key)
    {
        if (root == nil)
        {
            return nil;
        }

        if (root->key < key)
        {
            return lower_bound(nil, root->right, key);
        }

        auto bound = lower_bound(nil, root->left, key);
        if (bound != nil)
        {
            return bound;
        }
        return root;
    }

    static TreeRB* upper_bound(TreeRB* nil, TreeRB* root, const Key& key)
    {
        if (root == nil)
        {
            return nil;
        }

        if (!(key < root->key))
        {
            return upper_bound(nil, root->right, key);
        }

        auto bound = upper_bound(nil, root->left, key);
        if (bound != nil)
        {
            return bound;
        }
        return root;
    }

    static i32 black_height(TreeRB* nil, TreeRB* root)
    {
        i32 count = 0;
        while (root != nil)
        {
            if (root->color == Color::Black)
            {
                ++count;
            }

            root = root->left;
        }
        return count;
    }


    // NOTE : node != NIL, node->right != NIL
    static TreeRB* rotate_left(TreeRB* nil, TreeRB* root, TreeRB* node)
    {
        assert(node != nil);
        assert(node->right != nil);

        auto pivot = node->right;

        node->right = pivot->left;

        pivot->left = node;
        if (node->right != nil)
        {
            node->right->parent = node;
        }

        if (node->parent != nil)
        {
            if (node == node->parent->left)
            {
                node->parent->left = pivot;
            }
            else
            {
                node->parent->right = pivot;
            }
        }
        else
        {
            root = pivot;
        }

        pivot->parent = node->parent;
        node->parent = pivot;

        return root;
    }

    // NOTE : node != NIL, node->left != nullptr
    static TreeRB* rotate_right(TreeRB* nil, TreeRB* root, TreeRB* node)
    {
        assert(node != nil);
        assert(node->left != nil);

        auto pivot = node->left;

        node->left = pivot->right;
        pivot->right = node;
        if (node->left != nil)
        {
            node->left->parent = node;
        }

        if (node->parent != nil)
        {
            if (node->parent->left == node)
            {
                node->parent->left = pivot;
            }
            else
            {
                node->parent->right = pivot;
            }
        }
        else
        {
            root = pivot;
        }

        pivot->parent = node->parent;
        node->parent = pivot;

        return root;
    }


    // NOTE : node != NIL, node in default state except key
    static TreeRB* insert(TreeRB* nil, TreeRB* root, TreeRB* node)
    {
        assert(node != nil);

        auto prev = search_insert(nil, root, node->key);

        node->parent = prev;
        if (prev == nil)
        {
            root = node;
        }
        else
        {
            if (node->key < prev->key)
            {
                prev->left = node;
            }
            else
            {
                prev->right = node;
            }
        }
        node->color = Color::Red;
        node->left = nil;
        node->right = nil;

        return fix_insert(nil, root, node);
    }

    // NOTE : root != nil, after != nil, node != nil
    static TreeRB* insert_after(TreeRB* nil, TreeRB* root, TreeRB* after, TreeRB* node)
    {        
        assert(root != nil);
        assert(after != nil);
        assert(node != nil);

        if (after->right != nil)
        {
            TreeRB* succ = successor(nil, after);

            succ->left = node;
            node->parent = succ;
        }
        else
        {
            after->right = node;
            node->parent = after;
        }
        node->color = Color::Red;
        node->left  = nil;
        node->right = nil;

        return fix_insert(nil, root, node);
    }

    // NOTE : root != nil, before != nil, node != nil
    static TreeRB* insert_before(TreeRB* nil, TreeRB* root, TreeRB* before, TreeRB* node)
    {
        assert(root != nil);
        assert(before != nil);
        assert(node != nil);

        if (before->left != nil)
        {
            TreeRB* pred = predecessor(nil, before);

            pred->right= node;
            node->parent = pred;
        }
        else
        {
            before->left= node;
            node->parent = before;
        }
        node->color = Color::Red;
        node->left  = nil;
        node->right = nil;

        return fix_insert(nil, root, node);
    }

    static TreeRB* search_insert(TreeRB* nil, TreeRB* root, const Key& key)
    {
        TreeRB* prev = nil;
        TreeRB* curr = root;
        while (curr != nil)
        {
            prev = curr;
            if (key < curr->key)
            {
                curr = curr->left;
            }
            else 
            {
                curr = curr->right;
            }
        }
        return prev;
    }

    static TreeRB* fix_insert(TreeRB* nil, TreeRB* root, TreeRB* node)
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

                        root = rotate_left(nil, root, node);
                    }

                    node->parent->color = Color::Black;
                    node->parent->parent->color = Color::Red;

                    root = rotate_right(nil, root, node->parent->parent);
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

                        root = rotate_right(nil, root, node);
                    }

                    node->parent->color = Color::Black;
                    node->parent->parent->color = Color::Red;

                    root = rotate_left(nil, root, node->parent->parent);
                }
            }
        }

        root->color = Color::Black;

        return root;
    }


    static TreeRB* remove(TreeRB* nil, TreeRB* root, TreeRB* node)
    {
        assert(node != nil);

        Color removed = node->color;
        TreeRB* restore = nil;

        if (node->left == nil)
        {
            restore = node->right;

            root = transplant(nil, root, node, node->right);
        }
        else if (node->right == nil)
        {
            restore = node->left;

            root = transplant(nil, root, node, node->left);
        }
        else
        {
            auto leftmost = tree_min(nil, node->right);

            removed = leftmost->color;
            restore = leftmost->right;
            if (restore == nil)
            {
                restore->parent = leftmost;
            }

            if (node->right != leftmost)
            {
                root = transplant(nil, root, leftmost, leftmost->right);

                leftmost->right = node->right;
                node->right->parent = leftmost;
            }

            leftmost->left = node->left;
            node->left->parent = leftmost;

            root = transplant(nil, root, node, leftmost);

            leftmost->color = node->color;
        }

        if (removed == Color::Black)
        {
            root = fix_remove(nil, root, restore);
        }

        node->parent = nullptr;
        node->left   = nullptr;
        node->right  = nullptr;

        return root;
    }

    static TreeRB* transplant(TreeRB* nil, TreeRB* root, TreeRB* node, TreeRB* trans)
    {
        if (node->parent != nil)
        {
            if (node == node->parent->left)
            {
                node->parent->left = trans;
            }
            else
            {
                node->parent->right = trans;
            }
        }
        else
        {
            root = trans;
        }

        trans->parent = node->parent;

        return root;
    }

    static TreeRB* fix_remove(TreeRB* nil, TreeRB* root, TreeRB* restore)
    {
        while (restore != root && restore->color == Color::Black)
        {
            if (restore == restore->parent->left)
            {
                auto brother = restore->parent->right;
                if (brother->color == Color::Red)
                {
                    brother->color = Color::Black;
                    restore->parent->color = Color::Red;

                    root = rotate_left(nil, root, restore->parent);

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

                        root = rotate_right(nil, root, brother);

                        brother = restore->parent->right;
                    }

                    brother->color = restore->parent->color;
                    restore->parent->color = Color::Black;
                    brother->right->color = Color::Black;

                    root = rotate_left(nil, root, restore->parent);

                    restore = root;
                }
            }
            else
            {
                auto brother = restore->parent->left;
                if (brother->color == Color::Red)
                {
                    brother->color = Color::Black;
                    restore->parent->color = Color::Red;

                    root = rotate_right(nil, root, restore->parent);

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

                        root = rotate_left(nil, root, brother);

                        brother = restore->parent->left;
                    }

                    brother->color = restore->parent->color;
                    restore->parent->color = Color::Black;
                    brother->left->color = Color::Black;

                    root = rotate_right(nil, root, restore->parent);

                    restore = root;
                }
            }
        }

        restore->color = Color::Black;

        return root;
    }


    static TreeRB* find(TreeRB* nil, TreeRB* root, u32 key)
    {
        TreeRB* curr = root;
        while (curr != nil && curr->key != key)
        {
            if (key < curr->key)
            {
                curr = curr->left;
            }
            else
            {
                curr = curr->right;
            }
        }
        return curr;
    }


    Key key{};

    TreeRB* parent{};
    TreeRB* left{};
    TreeRB* right{};

    Color color{Color::Black};
};


namespace trb
{
    enum class Color : char
    {
        Black = 0,
        Red = 1,
    };

    template<class key_t, bool threaded_v>
    struct NodeTraits
    {
        static constexpr bool threaded = threaded_v;

        using Key = key_t;
    };

    template<class Traits, class = void>
    struct TreeNode;

    template<class Traits>
    struct TreeNode<Traits, std::enable_if_t<Traits::threaded>>
    {
        using Key = typename Traits::Key;

        Key key{};

        TreeNode* parent{};
        TreeNode* left{};
        TreeNode* right{};

        TreeNode* prev{};
        TreeNode* next{};

        Color color{Color::Black};
    };

    template<class Traits>
    struct TreeNode<Traits, std::enable_if_t<!Traits::threaded>>
    {
        using Key = typename Traits::Key;
        
        Key key{};

        TreeNode* parent{};
        TreeNode* left{};
        TreeNode* right{};

        Color color{Color::Black};
    };

    
    // TODO : tree class
    template<class NodeT>
    NodeT* tree_min(NodeT* nil, NodeT* root)
    {
        while (root->left != nil)
            root = root->left;
        return root;
    }   

    template<class NodeT>
    NodeT* tree_max(NodeT* nil, NodeT* root)
    {
        while (root->right != nil)
            root = root->right;
        return root;
    }

    template<class NodeT>
    NodeT* predecessor(NodeT* nil, NodeT* node)
    {
        if constexpr(!NodeT::threaded)
        {
            if (node->left != nil)
                return tree_max(nil, node->left);

            NodeT* pred = node->parent;
            while (pred != nil && node == pred->left)
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

    template<class NodeT>
    NodeT* successor(NodeT* nil, NodeT* node)
    {
        if constexpr(!NodeT::threaded)
        {
            if (node->right != nil)
            {
                return tree_min(nil, node->right);
            }

            NodeT* succ = node->parent;
            while (succ != nil && node == succ->right)
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

    template<class NodeT, class Key>
    NodeT* lower_bound(NodeT* nil, NodeT* root, Key&& key)
    {
        NodeT* lb = nil;
        while (root != nil)
        {
            if (root->key < key)
            {
                root = root->right;
            }
            else
            {
                lb = root;
                root = root->left;
            }
        }
        return lb;
    }

    template<class NodeT, class Key>
    NodeT* upper_bound(NodeT* nil, NodeT* root, Key&& key)
    {
        NodeT* ub = nil;
        while (root != nil)
        {
            if (!(key < root->key))
            {
                root = root->right;
            }
            else
            {
                ub = root;
                root = root->left;
            }
        }
        return ub;
    }

    template<class NodeT>
    u32 black_height(NodeT* nil, NodeT* root)
    {
        u32 count = 0;
        while (root != nil)
        {
            if (root->color == Color::Black)
                ++count;
            root = root->left;
        }
        return count;
    }


    // NOTE : node != NIL, node->right != NIL
    template<class NodeT>
    NodeT* rotate_left(NodeT* nil, NodeT* root, NodeT* node)
    {
        assert(node != nil);
        assert(node->right != nil);

        auto pivot = node->right;

        node->right = pivot->left;

        pivot->left = node;
        if (node->right != nil)
            node->right->parent = node;

        if (node->parent != nil)
        {
            if (node == node->parent->left)
                node->parent->left = pivot;            
            else
                node->parent->right = pivot;
        }
        else
        {
            root = pivot;
        }

        pivot->parent = node->parent;
        node->parent = pivot;

        return root;
    }

    // NOTE : node != NIL, node->left != nullptr
    template<class NodeT>
    NodeT* rotate_right(NodeT* nil, NodeT* root, NodeT* node)
    {
        assert(node != nil);
        assert(node->left != nil);

        auto pivot = node->left;

        node->left = pivot->right;
        pivot->right = node;
        if (node->left != nil)
            node->left->parent = node;

        if (node->parent != nil)
        {
            if (node->parent->left == node)
                node->parent->left = pivot;
            else
                node->parent->right = pivot;
        }
        else
        {
            root = pivot;
        }

        pivot->parent = node->parent;
        node->parent = pivot;

        return root;
    }


    template<class NodeT>
    NodeT* fix_insert(NodeT* nil, NodeT* root, NodeT* node)
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

                        root = rotate_left(nil, root, node);
                    }

                    node->parent->color = Color::Black;
                    node->parent->parent->color = Color::Red;

                    root = rotate_right(nil, root, node->parent->parent);
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

                        root = rotate_right(nil, root, node);
                    }

                    node->parent->color = Color::Black;
                    node->parent->parent->color = Color::Red;

                    root = rotate_left(nil, root, node->parent->parent);
                }
            }
        }

        root->color = Color::Black;

        return root;
    }

    // NOTE : node != NIL, node in default state except key
    template<class NodeT, class Compare, std::enable_if_t<!NodeT::threaded, int> = 0>
    NodeT* insert(NodeT* nil, NodeT* root, NodeT* node, Compare&& compare)
    {
        assert(node != nil);

        auto prev = search_insert(nil, root, node->key);

        node->parent = prev;
        if (prev == nil)
        {
            root = node;
        }
        else
        {
            if (compare(node->key, prev->key))
                prev->left = node;
            else
                prev->right = node;
        }
        node->color = Color::Red;
        node->left = nil;
        node->right = nil;

        return fix_insert(nil, root, node);
    }

    // NOTE : node != NIL, node in default state except key
    template<class NodeT, class Compare, std::enable_if_t<NodeT::threaded, int> = 0>
    NodeT* insert(NodeT* nil, NodeT* root, NodeT* node, Compare&& compare)
    {
        assert(node != nil);

        auto prev = search_insert(nil, root, node->key);

        node->parent = prev;
        if (prev == nil)
        {
            root = node;

            // list insert
            node->prev = nil;
            node->next = nil;

            nil->next = node;
            nil->prev = node;
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
        node->left  = nil;
        node->right = nil;

        node->color = Color::Red;

        return fix_insert(nil, root, node);
    }

    // NOTE : root != nil, after != nil, node != nil
    template<class NodeT>
    NodeT* insert_after(NodeT* nil, NodeT* root, NodeT* after, NodeT* node)
    {        
        assert(root != nil);
        assert(after != nil);
        assert(node != nil);

        if (after->right != nil)
        {
            NodeT* succ = successor(nil, after);

            succ->left = node;
            node->parent = succ;
        }
        else
        {
            after->right = node;
            node->parent = after;
        }
        node->color = Color::Red;
        node->left  = nil;
        node->right = nil;

        if constexpr(NodeT::threaded)
        {
            node->next = after->next;
            node->next->prev = node;
            node->prev = after;
            after->next = node;
        }

        return fix_insert(nil, root, node);
    }

    // NOTE : root != nil, before != nil, node != nil
    template<class NodeT>
    NodeT* insert_before(NodeT* nil, NodeT* root, NodeT* before, NodeT* node)
    {
        assert(root != nil);
        assert(before != nil);
        assert(node != nil);

        if (before->left != nil)
        {
            NodeT* pred = predecessor(nil, before);

            pred->right= node;
            node->parent = pred;
        }
        else
        {
            before->left= node;
            node->parent = before;
        }
        node->color = Color::Red;
        node->left  = nil;
        node->right = nil;

        if constexpr(NodeT::threaded)
        {
            node->prev = before->prev;
            node->prev->next = node;
            node->next = before;
            before->prev = node;
        }

        return fix_insert(nil, root, node);
    }

    template<class NodeT, class Key>
    NodeT* search_insert(NodeT* nil, NodeT* root, Key&& key)
    {
        NodeT* prev = nil;
        NodeT* curr = root;
        while (curr != nil)
        {
            prev = curr;
            if (key < curr->key)
                curr = curr->left;
            else 
                curr = curr->right;
        }
        return prev;
    }


    template<class NodeT, std::enable_if_t<!NodeT::threaded, int> = 0>
    NodeT* remove(NodeT* nil, NodeT* root, NodeT* node)
    {
        assert(node != nil);

        Color removed = node->color;
        NodeT* restore = nil;

        if (node->left == nil)
        {
            restore = node->right;

            root = transplant(nil, root, node, node->right);
        }
        else if (node->right == nil)
        {
            restore = node->left;

            root = transplant(nil, root, node, node->left);
        }
        else
        {
            auto leftmost = tree_min(nil, node->right);

            removed = leftmost->color;
            restore = leftmost->right;
            if (restore == nil)
            {
                restore->parent = leftmost;
            }

            if (node->right != leftmost)
            {
                root = transplant(nil, root, leftmost, leftmost->right);

                leftmost->right = node->right;
                node->right->parent = leftmost;
            }

            leftmost->left = node->left;
            node->left->parent = leftmost;

            root = transplant(nil, root, node, leftmost);

            leftmost->color = node->color;
        }

        if (removed == Color::Black)
        {
            root = fix_remove(nil, root, restore);
        }

        //node->parent = nullptr;
        //node->left   = nullptr;
        //node->right  = nullptr;

        nil->parent = nil;

        return root;
    }

    template<class NodeT, std::enable_if_t<NodeT::threaded, int> = 0>
    NodeT* remove(NodeT* nil, NodeT* root, NodeT* node)
    {
        assert(node != nil);

        Color removed = node->color;
        NodeT* restore = nil;

        if (node->left == nil)
        {
            restore = node->right;

            root = transplant(nil, root, node, node->right);
        }
        else if (node->right == nil)
        {
            restore = node->left;

            root = transplant(nil, root, node, node->left);
        }
        else
        {
            //auto leftmost = tree_min(nil, node->right);
            auto leftmost = node->next;

            removed = leftmost->color;
            restore = leftmost->right;
            if (restore == nil)
            {
                restore->parent = leftmost;
            }

            if (node->right != leftmost)
            {
                root = transplant(nil, root, leftmost, leftmost->right);

                leftmost->right = node->right;
                node->right->parent = leftmost;
            }

            leftmost->left = node->left;
            node->left->parent = leftmost;

            root = transplant(nil, root, node, leftmost);

            leftmost->color = node->color;
        }

        if (removed == Color::Black)
        {
            root = fix_remove(nil, root, restore);
        }

        // list remove
        node->prev->next = node->next;
        node->next->prev = node->prev;

        //node->parent = nullptr;
        //node->left   = nullptr;
        //node->right  = nullptr;

        // list
        //node->prev   = nullptr;
        //node->next   = nullptr;

        nil->parent = nil;

        return root;
    }

    template<class NodeT>
    NodeT* transplant(NodeT* nil, NodeT* root, NodeT* node, NodeT* trans)
    {
        if (node->parent != nil)
        {
            if (node == node->parent->left)
                node->parent->left = trans;
            else
                node->parent->right = trans;
        }
        else
        {
            root = trans;
        }

        trans->parent = node->parent;

        return root;
    }

    template<class NodeT>
    NodeT* fix_remove(NodeT* nil, NodeT* root, NodeT* restore)
    {
        while (restore != root && restore->color == Color::Black)
        {
            if (restore == restore->parent->left)
            {
                auto brother = restore->parent->right;
                if (brother->color == Color::Red)
                {
                    brother->color = Color::Black;
                    restore->parent->color = Color::Red;

                    root = rotate_left(nil, root, restore->parent);

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

                        root = rotate_right(nil, root, brother);

                        brother = restore->parent->right;
                    }

                    brother->color = restore->parent->color;
                    restore->parent->color = Color::Black;
                    brother->right->color = Color::Black;

                    root = rotate_left(nil, root, restore->parent);

                    restore = root;
                }
            }
            else
            {
                auto brother = restore->parent->left;
                if (brother->color == Color::Red)
                {
                    brother->color = Color::Black;
                    restore->parent->color = Color::Red;

                    root = rotate_right(nil, root, restore->parent);

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

                        root = rotate_left(nil, root, brother);

                        brother = restore->parent->left;
                    }

                    brother->color = restore->parent->color;
                    restore->parent->color = Color::Black;
                    brother->left->color = Color::Black;

                    root = rotate_right(nil, root, restore->parent);

                    restore = root;
                }
            }
        }

        restore->color = Color::Black;

        return root;
    }


    template<class NodeT, class Compare, class Key>
    NodeT* find(NodeT* nil, NodeT* root, Compare&& compare, Key&& key)
    {
        NodeT* curr = root;
        while (curr != nil && curr->key != key)
        {
            if (compare(key, curr->key))
                curr = curr->left;            
            else
                curr = curr->right;
        }
        return curr;
    }


    // TODO : allocator
    template<class Key, class Compare = std::less<Key>>
    class Set
    {
        friend class Iterator;

    public:
        using Traits = NodeTraits<Key, false>;
        using Node   = TreeNode<Traits>;

        class Iterator
        {
            friend class Set;

        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = Key;
            using reference = Key&;
            using pointer = Key*;
            using difference_type = std::ptrdiff_t;

        private:
            Iterator(Set& set, Node* node) : m_set{&set}, m_node{node}
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
                m_node = predecessor(m_set->m_nil, m_node);

                return *this;
            }

            Iterator& operator ++ ()
            {
                m_node = successor(m_set->m_nil, m_node);

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
                    while (m_set->m_nil != m_node && diff > 0) {
                        ++it;
                        --diff;
                    }
                }
                else {
                    while (m_set->m_nil != m_node && diff < 0) {
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
            Set* m_set{nullptr};
            Node* m_node{nullptr};
        };


    public:
        // TODO : it's a simplified version
        template<class Comp = Compare, std::enable_if_t<std::is_default_constructible_v<Comp>, int> = 0>
        Set(Comp&& comp = Comp()) : m_compare(std::forward<Comp>(comp))
        {}

        // TODO : it's a simplified version
        template<class Comp = Compare, std::enable_if_t<!std::is_default_constructible_v<Comp>, int> = 0>
        Set(Comp&& comp) : m_compare(std::forward<Comp>(comp))
        {}


    public:
        

    private:
        Node* m_nil{};
        Node* root{};
        Compare m_compare;
    };
}
