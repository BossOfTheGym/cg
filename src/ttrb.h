#pragma once

#include "core.h"

#include <iostream>
#include <memory>


// TODO : create simple template
// TODO : iterator
// TODO : join, split
struct ThreadedTreeRB
{
    enum class Color : char
    {
        Black = 0,
        Red = 1,
    };

    using Key = u32;


    static ThreadedTreeRB* tree_min(ThreadedTreeRB* nil, ThreadedTreeRB* root)
    {
        return nil->next;
    }

    static ThreadedTreeRB* tree_max(ThreadedTreeRB* nil, ThreadedTreeRB* root)
    {
        return nil->prev;
    }


    // NOTE : node != NIL, node->right != NIL
    static ThreadedTreeRB* rotate_left(ThreadedTreeRB* nil, ThreadedTreeRB* root, ThreadedTreeRB* node)
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
    static ThreadedTreeRB* rotate_right(ThreadedTreeRB* nil, ThreadedTreeRB* root, ThreadedTreeRB* node)
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
    static ThreadedTreeRB* insert(ThreadedTreeRB* nil, ThreadedTreeRB* root, ThreadedTreeRB* node)
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
            if (node->key < prev->key)
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

    static ThreadedTreeRB* search_insert(ThreadedTreeRB* nil, ThreadedTreeRB* root, const Key& key)
    {
        ThreadedTreeRB* prev = nil;
        ThreadedTreeRB* curr = root;
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

    static ThreadedTreeRB* fix_insert(ThreadedTreeRB* nil, ThreadedTreeRB* root, ThreadedTreeRB* node)
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


    static ThreadedTreeRB* remove(ThreadedTreeRB* nil, ThreadedTreeRB* root, ThreadedTreeRB* node)
    {
        assert(node != nil);

        Color removed = node->color;
        ThreadedTreeRB* restore = nil;

        // list remove
        node->prev->next = node->next;
        node->next->prev = node->prev;

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

        node->prev   = nullptr;
        node->next   = nullptr;

        return root;
    }

    static ThreadedTreeRB* transplant(ThreadedTreeRB* nil, ThreadedTreeRB* root, ThreadedTreeRB* node, ThreadedTreeRB* trans)
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

    static ThreadedTreeRB* fix_remove(ThreadedTreeRB* nil, ThreadedTreeRB* root, ThreadedTreeRB* restore)
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


    static ThreadedTreeRB* find(ThreadedTreeRB* nil, ThreadedTreeRB* root, u32 key)
    {
        ThreadedTreeRB* curr = root;
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


    static ThreadedTreeRB* predecessor(ThreadedTreeRB* nil, ThreadedTreeRB* node)
    {
        return node->prev;
    }

    static ThreadedTreeRB* successor(ThreadedTreeRB* nil, ThreadedTreeRB* node)
    {
        return node->next;
    }

    static ThreadedTreeRB* lower_bound(ThreadedTreeRB* nil, ThreadedTreeRB* root, const Key& key)
    {
        ThreadedTreeRB* lb = nil;
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

    static ThreadedTreeRB* upper_bound(ThreadedTreeRB* nil, ThreadedTreeRB* root, const Key& key)
    {
        ThreadedTreeRB* ub = nil;
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

    static i32 black_height(ThreadedTreeRB* nil, ThreadedTreeRB* root)
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
    static void dfs(ThreadedTreeRB* nil, ThreadedTreeRB* root)
    {
        if (root != nil)
        {
            dfs(nil, root->left);
            std::cout << "(" 
                << (root->parent != nil ? (int)root->parent->key : -1) << " "
                << (root->prev != nil ? (int)root->prev->key : -1) << " "
                << (root->next != nil ? (int)root->next->key : -1) << " | "
                << root->key << " " 
                << (int)root->color << ") ";
            dfs(nil, root->right);
        }
    }

    // DEBUG
    static i32 invariant(ThreadedTreeRB* nil, ThreadedTreeRB* root)
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
    static bool valid_nil(ThreadedTreeRB* nil)
    {
        return nil->left == nil
            && nil->right == nil
            && nil->color == Color::Black;
    }


    Key key{};

    ThreadedTreeRB* parent{};
    ThreadedTreeRB* left{};
    ThreadedTreeRB* right{};

    ThreadedTreeRB* prev{};
    ThreadedTreeRB* next{};

    Color color{Color::Black};
};


// DEBUG
struct MyThreadedSet
{
    MyThreadedSet()
    {
        nil = std::make_unique<ThreadedTreeRB>();
        nil->left  = nil.get();
        nil->right = nil.get();
        nil->prev  = nil.get();
        nil->next  = nil.get();

        nil->color = ThreadedTreeRB::Color::Black;

        root = nil.get();
    }

    ~MyThreadedSet()
    {
        clear();
    }

    void add(u32 key)
    {
        root = ThreadedTreeRB::insert(nil.get(), root, new ThreadedTreeRB{key});
    }

    bool has(u32 key)
    {
        return ThreadedTreeRB::find(nil.get(), root, key) != nil.get();
    }

    void remove(u32 key)
    {
        if (auto node = ThreadedTreeRB::find(nil.get(), root, key); node != nil.get())
        {
            root = ThreadedTreeRB::remove(nil.get(), root, node);

            delete node;
        }
    }

    void clear()
    {
        clearImpl(root);

        root = nil.get();
        nil->prev = nil.get();
        nil->next = nil.get();
        nil->parent = nullptr;
    }

    void clearImpl(ThreadedTreeRB* node)
    {
        if (node != nil.get())
        {
            clearImpl(node->left);
            clearImpl(node->right);

            delete node;
        }
    }


    ThreadedTreeRB* lowerBound(u32 key)
    {
        return ThreadedTreeRB::lower_bound(nil.get(), root, key);
    }

    ThreadedTreeRB* upperBound(u32 key)
    {
        return ThreadedTreeRB::upper_bound(nil.get(), root, key);
    }

    ThreadedTreeRB* min()
    {
        return ThreadedTreeRB::tree_min(nil.get(), root);
    }

    ThreadedTreeRB* max()
    {
        return ThreadedTreeRB::tree_max(nil.get(), root);
    }

    ThreadedTreeRB* succ(ThreadedTreeRB* node)
    {
        return ThreadedTreeRB::successor(nil.get(), node);
    }

    ThreadedTreeRB* pred(ThreadedTreeRB* node)
    {
        return ThreadedTreeRB::predecessor(nil.get(), node);
    }


    bool isNil(ThreadedTreeRB* node)
    {
        return node == nil.get();
    }


    void dfs()
    {
        ThreadedTreeRB::dfs(nil.get(), root);

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
        return ThreadedTreeRB::invariant(nil.get(), root) != -1;
    }

    bool validNil()
    {
        return ThreadedTreeRB::valid_nil(nil.get());
    }


    bool empty()
    {
        return root == nil.get();
    }


    std::unique_ptr<ThreadedTreeRB> nil{};
    ThreadedTreeRB* root{};
};
