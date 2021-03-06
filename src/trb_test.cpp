#include "trb_test.h"

#include "core.h"
#include "trb_set.h"
#include "test_util.h"

#include <set>
#include <vector>
#include <random>
#include <numeric>
#include <cstdlib>
#include <iostream>
#include <algorithm>


using Set = ds::Set<u32>;


void test_set_stress()
{
    std::vector<u32> keys(2'000'000);
    std::iota(keys.begin(), keys.end(), 0);

    //std::random_device device;
    //std::minstd_rand gen(device());
    std::minstd_rand gen(12345);

    for (i32 k = 0; k < 10; k++)
    {
        shuffle(keys, gen);

        std::set<u32> s;

        auto c = clock();
        for (auto& key : keys)
        {
            s.insert(key);
        }
        c = clock() - c;

        std::cout << "set elapsed: " << (float)c / CLOCKS_PER_SEC << std::endl;   
    }
    std::cout << std::endl;
}

void test_my_set_stress()
{
    std::cout << "************************************" << std::endl;
    std::cout << "**** testing elements insertion ****" << std::endl;
    std::cout << "************************************" << std::endl;

    std::vector<u32> keys(2'000'000);
    std::iota(keys.begin(), keys.end(), 0);

    //std::random_device device;
    //std::minstd_rand gen(device());
    std::minstd_rand gen(12345);

    for (u32 k = 0; k < 10; k++)
    {
        shuffle(keys, gen);

        Set s;

        auto c = clock();
        for (auto& key : keys)
        {
            s.insert(key);
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

        Set ms;

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            std::cout << "(add) " << key << ": ";

            ms.insert(key);
            //ms.dfs();

            //assert(ms.invariant());
            //assert(ms.validNil());
        }

        //ms.dfs();

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            std::cout << "(rem) " << key << ": ";

            ms.erase(key);
            //ms.dfs();

            //assert(ms.invariant());
            //assert(ms.validNil());
        }

        assert(ms.empty());

        std::cout << std::endl;
    }

    std::cout << "testing ended" << std::endl << std::endl;
}

void test_structure_debug()
{
    Set s;

    for (u32 i = 0; i < 15; i++)
    {
        s.insert(i);
    }

    //s.dfs();
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

        Set ms;

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.insert(key);

            //assert(ms.invariant());
            //assert(ms.validNil());
        }

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.erase(key);

            //assert(ms.invariant());
            //assert(ms.validNil());
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

        Set ms;

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.insert(key);

            //assert(ms.invariant());
            //assert(ms.validNil());
        }

        shuffle(keys, gen);
        for (auto& key : keys)
        {
            ms.erase(key);

            //assert(ms.invariant());
            //assert(ms.validNil());
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
        ds::Multiset<u32> ms;

        auto c = clock();
        for (u32 i = 0; i < 1'000'000; i++)
        {
            ms.insert(0u);
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

    auto count = 256;

    std::vector<u32> keys(count * 3);
    std::iota(keys.begin(), keys.begin() + count, 0);
    std::iota(keys.begin() + count, keys.begin() + count * 2, 0);
    std::iota(keys.begin() + count, keys.begin() + count * 3, 0);

    std::random_device device;

    auto seed = device(); // 2860085261
    std::minstd_rand gen(seed);

    std::cout << "seed: " << seed << std::endl;

    std::vector<u32> added;
    std::vector<u32> removed;

    Set ms;

    for (i32 k = 0; k < 100'000; k++)
    {
        if (k % 1000 == 0)
        {
            std::cout << "test: " << k << std::endl;
        }

        shuffle(keys, gen);

        for (i32 c = 0 ; c < (i32)keys.size() / 2; c++)
        {
            ms.insert(keys[c]);

            //assert(ms.invariant());
            //assert(ms.validNil());
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

            ms.insert(key);

            //assert(ms.invariant());
            //assert(ms.validNil());
        };

        auto rem = [&] ()
        {
            choose(removed, gen);

            auto key = added.back();
            added.pop_back();

            removed.push_back(key);

            ms.erase(key);

            //assert(ms.invariant());
            //assert(ms.validNil());
        };

        for (i32 m = 0; m < 5'000; m++)
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

        Set myset;
        for (auto& key : keys)
        {
            myset.insert(key);
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
            if (lb1 != myset.end())
            {
                std::cout << "myset: " << *lb1 << std::endl;
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

        Set myset;
        for (auto& key : keys)
        {
            myset.insert(key);
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
            if (lb1 != myset.end())
            {
                std::cout << "myset: " << *lb1 << std::endl;
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

    Set ms;
    for (u32 k = 0; k < 3; k++)
    {
        for(auto& key : keys)
        {
            ms.insert(key);
        }
    }

    std::cout << "Forward: ";
    auto it  = ms.begin();
    auto nil = ms.end();
    while (it != nil)
    {
        std::cout << *it << " ";

        ++it;
    }
    std::cout << std::endl;

    //std::cout << "Backward: ";
    //it  = ms.max();
    //nil = ms.nil.get();
    //while (it != nil)
    //{
    //    std::cout << it->key << " ";
    //
    //    it = ms.pred(it);
    //}
    //std::cout << std::endl;

    std::cout << "testing ended" << std::endl << std::endl;
}


void test_multiset_order()
{
    std::cout << "********************************" << std::endl;
    std::cout << "**** testing multiset order ****" << std::endl;
    std::cout << "********************************" << std::endl;

    using Pair = std::pair<u32, u32>;

    struct Comp
    {
        bool operator () (const Pair& p0, const Pair& p1)
        {
            return p0.first < p1.first;
        }
    };

    using Multiset = ds::Multiset<Pair, Comp>;

    Multiset ms;
    
    std::random_device device;
    std::minstd_rand gen(device());
    
    std::vector<Pair> pairs;
    for (u32 i = 0; i < 5; i++)
    {
        pairs.push_back({i, 0});
    }

    auto choose = [&] ()
    {
        auto i = gen() % pairs.size();
        auto pair = pairs[i];
        if (++pairs[i].second >= 5)
        {
            std::swap(pairs[i], pairs.back());
            pairs.pop_back();
        }
        return pair;
    };

    while (!pairs.empty())
    {
        ms.insert(choose());
    }

    for (auto& [k, o] : ms)
    {
        std::cout << "(" << k << ":" << o << ") ";
    }

    std::cout << "Testing finished." << std::endl << std::endl;
}
