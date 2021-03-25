#pragma once

#include "core.h"

#include <iostream>
#include <memory>


// TODO : create simple template
// TODO : iterator
// TODO : join, split
template<class Key>
struct ThreadedTreeRB
{
    enum class Color : char
    {
        Black = 0,
        Red = 1,
    };

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
    // NOTE : requires compare
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

    // NOTE : requires compare
    static ThreadedTreeRB* search_insert(ThreadedTreeRB* nil, ThreadedTreeRB* root, const Key& key)
    {
        ThreadedTreeRB* prev = nil;
        ThreadedTreeRB* curr = root;
        while (curr != nil)
        {
            prev = curr;
            if (key < curr->key) // compare
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
            // auto leftmost = tree_min(nil, node->right);
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

        node->parent = nullptr;
        node->left   = nullptr;
        node->right  = nullptr;

        // list
        node->prev->next = node->next;
        node->next->prev = node->prev;

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

    // NOTE : requires compare
    static ThreadedTreeRB* find(ThreadedTreeRB* nil, ThreadedTreeRB* root, const Key& key)
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

    // NOTE : requires compare
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

    // NOTE : requires compare
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


    Key key{};

    ThreadedTreeRB* parent{};
    ThreadedTreeRB* left{};
    ThreadedTreeRB* right{};

    ThreadedTreeRB* prev{};
    ThreadedTreeRB* next{};

    Color color{Color::Black};
};
