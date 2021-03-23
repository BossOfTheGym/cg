#pragma once

#include "core.h"

#include <iostream>
#include <memory>


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


    // DEBUG
    static void dfs(TreeRB* nil, TreeRB* root)
    {
        if (root != nil)
        {
            dfs(nil, root->left);
            std::cout << "(" << (root->parent != nil ? (int)root->parent->key : -1) << " " << root->key << " " << (int)root->color << ") ";
            dfs(nil, root->right);
        }
    }

    // DEBUG
    static i32 invariant(TreeRB* nil, TreeRB* root)
    {
        if (root != nil)
        {
            auto invL = invariant(nil, root->left);
            auto invR = invariant(nil, root->right);
            if (invL == -1 || invR == -1 || invL != invR)
            {
                return -1;
            }

            return (root->color == Color::Black ? 1 : 0) + invL;
        }

        return 0;
    }

    // DEBUG
    static bool valid_nil(TreeRB* nil)
    {
        return nil->left  == nil
            && nil->right == nil
            && nil->color == Color::Black;
    }


    Key key{};

    TreeRB* parent{};
    TreeRB* left{};
    TreeRB* right{};

    Color color{Color::Black};
};


// DEBUG
struct MySet
{
    MySet()
    {
        nil = std::make_unique<TreeRB>();
        nil->left = nil.get();
        nil->right = nil.get();
        nil->color = TreeRB::Color::Black;

        root = nil.get();
    }

    ~MySet()
    {
        clear();
    }

    void add(u32 key)
    {
        root = TreeRB::insert(nil.get(), root, new TreeRB{key});
    }

    bool has(u32 key)
    {
        return TreeRB::find(nil.get(), root, key) != nil.get();
    }

    void remove(u32 key)
    {
        if (auto node = TreeRB::find(nil.get(), root, key); node != nil.get())
        {
            root = TreeRB::remove(nil.get(), root, node);

            delete node;
        }
    }

    void clear()
    {
        clearImpl(root);

        root = nil.get();
    }

    void clearImpl(TreeRB* node)
    {
        if (node != nil.get())
        {
            clearImpl(node->left);
            clearImpl(node->right);

            delete node;
        }
    }


    TreeRB* lowerBound(u32 key)
    {
        return TreeRB::lower_bound(nil.get(), root, key);
    }

    TreeRB* upperBound(u32 key)
    {
        return TreeRB::upper_bound(nil.get(), root, key);
    }

    TreeRB* min()
    {
        return TreeRB::tree_min(nil.get(), root);
    }

    TreeRB* max()
    {
        return TreeRB::tree_max(nil.get(), root);
    }

    TreeRB* succ(TreeRB* node)
    {
        return TreeRB::successor(nil.get(), node);
    }

    TreeRB* pred(TreeRB* node)
    {
        return TreeRB::predecessor(nil.get(), node);
    }


    bool isNil(TreeRB* node)
    {
        return node == nil.get();
    }


    void dfs()
    {
        TreeRB::dfs(nil.get(), root);

        std::cout << std::endl;
    }

    void traverse()
    {
        auto it = min();
        while (it != nil.get())
        {
            it = succ(it);
        }

        it = max();
        while (it != nil.get())
        {
            it = pred(it);
        }
    }

    bool invariant()
    {
        return TreeRB::invariant(nil.get(), root) != -1;
    }

    bool validNil()
    {
        return TreeRB::valid_nil(nil.get());
    }


    bool empty()
    {
        return root == nil.get();
    }


    std::unique_ptr<TreeRB> nil{};
    TreeRB* root{};
};
