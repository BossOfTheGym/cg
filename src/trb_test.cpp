#include "trb_test.h"

#include "core.h"
#include "test_util.h"
#include "trb.h"

#include <cstdlib>
#include <vector>
#include <algorithm>
#include <random>
#include <set>
#include <iostream>
#include <numeric>


namespace
{
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

            return (root->color == TreeRB::Color::Black ? 1 : 0) + invL;
        }

        return 0;
    }

    // DEBUG
    static bool valid_nil(TreeRB* nil)
    {
        return nil->left  == nil
            && nil->right == nil
            && nil->color == TreeRB::Color::Black;
    }
}

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
        ::dfs(nil.get(), root);

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
        return ::invariant(nil.get(), root) != -1;
    }

    bool validNil()
    {
        return valid_nil(nil.get());
    }


    bool empty()
    {
        return root == nil.get();
    }


    std::unique_ptr<TreeRB> nil{};
    TreeRB* root{};
};

void test_set_stress()
{
    std::set<u32> s;

    auto c = clock();
    for (u32 i = 0; i < 1'000'000; i++)
    {
        s.insert(i);
    }
    c = clock() - c;

    std::cout << "set elapsed: " << (float)c / CLOCKS_PER_SEC << std::endl << std::endl;
}

void test_my_set_stress()
{
    std::cout << "************************************" << std::endl;
    std::cout << "**** testing elements insertion ****" << std::endl;
    std::cout << "************************************" << std::endl;

    for (u32 k = 0; k < 3; k++)
    {
        MySet s;

        auto c = clock();
        for (u32 i = 0; i < 1'000'000; i++)
        {
            s.add(i);
        }
        c = clock() - c;

        std::cout << "my set elapsed: " << (f32)c / CLOCKS_PER_SEC << std::endl;
    }

    std::cout << std::endl;
}


void test_my_set_debug()
{
    std::vector<u32> keys(15);
    std::iota(keys.begin(), keys.end(), 0);

    std::random_device device;
    std::minstd_rand gen(12345);

    for (i32 k = 0; k < 10; k++)
    {
        std::cout << "test: " << k << std::endl;

        MySet ms;

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            std::cout << "(add) " << key << ": ";

            ms.add(key);
            ms.dfs();

            assert(ms.invariant());
            assert(ms.validNil());
        }

        ms.dfs();

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            std::cout << "(rem) " << key << ": ";

            ms.remove(key);
            ms.dfs();

            assert(ms.invariant());
            assert(ms.validNil());
        }

        assert(ms.empty());

        std::cout << std::endl;
    }

    std::cout << "testing ended" << std::endl << std::endl;
}

void test_structure_debug()
{
    MySet s;

    for (u32 i = 0; i < 15; i++)
    {
        s.add(i);
    }

    s.dfs();
}


void test_my_set0()
{
    std::vector<u32> keys(10);
    std::iota(keys.begin(), keys.end(), 0);

    std::random_device device;
    //std::minstd_rand gen(11);
    std::minstd_rand gen(device());

    std::cout << "********************************" << std::endl;
    std::cout << "**** testing small-size set ****" << std::endl;
    std::cout << "********************************" << std::endl;
    for (i32 k = 0; k < 1'000'000; k++)
    {
        if (k % 10'000 == 0)
        {
            std::cout << "test: " << k << std::endl;
        }

        MySet ms;

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.add(key);

            assert(ms.invariant());
            assert(ms.validNil());
        }

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.remove(key);

            assert(ms.invariant());
            assert(ms.validNil());
        }

        assert(ms.empty());
    }

    std::cout << "testing ended" << std::endl << std::endl;
}

void test_my_set1()
{
    std::vector<u32> keys(128);
    std::iota(keys.begin(), keys.end(), 0);

    std::random_device device;
    //std::minstd_rand gen(12345);
    std::minstd_rand gen(device());

    std::cout << "*********************************" << std::endl;
    std::cout << "**** testing middle-size set ****" << std::endl;
    std::cout << "*********************************" << std::endl;
    for (i32 k = 0; k < 1'00'000; k++)
    {
        if (k % 1'000 == 0)
        {
            std::cout << "test: " << k << std::endl;
        }

        MySet ms;

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.add(key);

            assert(ms.invariant());
            assert(ms.validNil());
        }

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.remove(key);

            assert(ms.invariant());
            assert(ms.validNil());
        }

        assert(ms.empty());
    }

    std::cout << "testing ended" << std::endl << std::endl;
}


void test_my_set_insert_equal()
{
    std::cout << "******************************************" << std::endl;
    std::cout << "**** testing equal elements insertion ****" << std::endl;
    std::cout << "******************************************" << std::endl;

    for (u32 k = 0; k < 3; k++)
    {
        MySet ms;

        auto c = clock();
        for (u32 i = 0; i < 1'000'000; i++)
        {
            ms.add(0);
        }
        c = clock() - c;
        std::cout << "elapsed: " << (f32)c / CLOCKS_PER_SEC << std::endl;
    }

    std::cout << std::endl;
}

void test_my_set_delete_insert()
{
    std::cout << "*******************************" << std::endl;
    std::cout << "**** testing insert-delete ****" << std::endl;
    std::cout << "*******************************" << std::endl;

    auto count = 50;

    std::vector<u32> keys(count * 3);
    std::iota(keys.begin(), keys.begin() + count, 0);
    std::iota(keys.begin() + count, keys.begin() + count * 2, 0);
    std::iota(keys.begin() + count, keys.begin() + count * 3, 0);

    std::random_device device;
    
    auto seed = device(); // 12345
    std::minstd_rand gen(seed);

    std::cout << "seed: " << seed << std::endl;

    std::vector<u32> added;
    std::vector<u32> removed;

    MySet ms;

    for (i32 k = 0; k < 10'000; k++)
    {
        if (k % 1000 == 0)
        {
            std::cout << "test: " << k << std::endl;
        }

        shuffle(keys, gen);

        for (i32 c = 0 ; c < (i32)keys.size() / 2; c++)
        {
            ms.add(keys[c]);

            assert(ms.invariant());
            assert(ms.validNil());
        }

        added.clear();
        removed.clear();

        added.insert(added.end(), keys.begin(), keys.begin() + keys.size() / 2);
        removed.insert(removed.end(), keys.begin() + keys.size() / 2, keys.end());


        auto add = [&] ()
        {
            choose(removed, gen);

            auto key = removed.back();
            removed.pop_back();

            added.push_back(key);

            ms.add(key);

            assert(ms.invariant());
            assert(ms.validNil());
        };

        auto rem = [&] ()
        {
            choose(removed, gen);

            auto key = added.back();
            added.pop_back();

            removed.push_back(key);

            ms.remove(key);

            assert(ms.invariant());
            assert(ms.validNil());
        };

        for (i32 m = 0; m < 1'000; m++)
        {
            auto action = gen() & 0x1;
            if (action == 0) // add
            {
                if (removed.empty())
                {
                    rem();
                }
                else
                {
                    add();
                }

                if (removed.empty())
                {
                    rem();
                }
                else
                {
                    add();
                }
            }
            else // remove
            {
                if (added.empty())
                {
                    add();
                }
                else
                {
                    rem();
                }
                if (added.empty())
                {
                    add();
                }
                else
                {
                    rem();
                }
            }
        }

        while(!added.empty())
        {
            rem();
        }

        assert(ms.empty());
    }
    std::cout << "testing ended" << std::endl << std::endl;
}



namespace
{
    void test_my_set_lower_bound(const std::vector<u32>& keys, u32 left, u32 right)
    {
        std::cout << "keys: ";
        for (auto key : keys)
        {
            std::cout << key << " ";
        }
        std::cout << std::endl;

        std::set<u32> stdset(keys.begin(), keys.end());

        MySet myset;
        for (auto& key : keys)
        {
            myset.add(key);
        }

        for (; left != right; left++)
        {
            u32 key = left;

            std::cout << "lower bound for key " << key << std::endl;

            auto lb0 = stdset.lower_bound(key);
            if (lb0 != stdset.end())
            {
                std::cout << "stdset: " << *lb0 << std::endl;
            }
            else
            {
                std::cout << "stdset: nil" << std::endl;
            }

            auto lb1 = myset.lowerBound(key);
            if (!myset.isNil(lb1))
            {
                std::cout << "myset: " << lb1->key << std::endl;
            }
            else
            {
                std::cout << "myset: nil" << std::endl;
            }

            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    void test_my_set_upper_bound(const std::vector<u32>& keys, u32 left, u32 right)
    {
        std::set<u32> stdset(keys.begin(), keys.end());

        MySet myset;
        for (auto& key : keys)
        {
            myset.add(key);
        }

        for (; left != right; left++)
        {
            u32 key = left;

            std::cout << "upper bound for key " << key << std::endl;

            auto lb0 = stdset.upper_bound(key);
            if (lb0 != stdset.end())
            {
                std::cout << "stdset: " << *lb0 << std::endl;
            }
            else
            {
                std::cout << "stdset: nil" << std::endl;
            }

            auto lb1 = myset.upperBound(key);
            if (!myset.isNil(lb1))
            {
                std::cout << "myset: " << lb1->key << std::endl;
            }
            else
            {
                std::cout << "myset: nil" << std::endl;
            }

            std::cout << std::endl;
        }
    }
}


void test_my_set_lower_bound()
{
    std::cout << "*****************************" << std::endl;
    std::cout << "**** testing lower_bound ****" << std::endl;
    std::cout << "*****************************" << std::endl;

    test_my_set_lower_bound({1,2,3,4,5,6,7,8,9,10}, 0, 11);
    test_my_set_lower_bound({2,4,6,8,10}, 0, 11);
    test_my_set_lower_bound({1,3,5,7,9}, 0, 11);

    std::cout << "testing finished" << std::endl << std::endl;
}

void test_my_set_upper_bound()
{
    std::cout << "*****************************" << std::endl;
    std::cout << "**** testing upper_bound ****" << std::endl;
    std::cout << "*****************************" << std::endl;

    test_my_set_upper_bound({1,2,3,4,5,6,7,8,9,10}, 0, 11);
    test_my_set_upper_bound({2,4,6,8,10}, 0, 11);
    test_my_set_upper_bound({1,3,5,7,9}, 0, 11);

    std::cout << "testing finished" << std::endl << std::endl;
}


void test_my_set_iteration()
{
    std::cout << "***************************" << std::endl;
    std::cout << "**** testing iteration ****" << std::endl;
    std::cout << "***************************" << std::endl;

    std::vector<u32> keys{1,2,3,4,5,6};

    MySet ms;
    for (u32 k = 0; k < 3; k++)
    {
        for(auto& key : keys)
        {
            ms.add(key);
        }
    }
    
    std::cout << "Forward: ";
    auto it  = ms.min();
    auto nil = ms.nil.get();
    while (it != nil)
    {
        std::cout << it->key << " ";

        it = ms.succ(it);
    }
    std::cout << std::endl;

    std::cout << "Backward: ";
    it  = ms.max();
    nil = ms.nil.get();
    while (it != nil)
    {
        std::cout << it->key << " ";

        it = ms.pred(it);
    }
    std::cout << std::endl;

    std::cout << "testing ended" << std::endl << std::endl;
}
