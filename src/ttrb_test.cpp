#include "ttrb_test.h"

#include "ttrb.h"
#include "core.h"
#include "test_util.h"

#include <set>
#include <vector>
#include <random>
#include <string>
#include <numeric>
#include <iostream>


using MySet = MyThreadedSet;
using namespace std::string_literals;

void test_threaded_0()
{
    std::vector<u32> keys(16);
    std::iota(keys.begin(), keys.end(), 0);

    std::random_device device;
    //std::minstd_rand gen(11);
    std::minstd_rand gen(device());

    std::cout << "*****************************************" << std::endl;
    std::cout << "**** testing threaded small-size set ****" << std::endl;
    std::cout << "*****************************************" << std::endl;
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

void test_threaded_1()
{
    std::cout << "******************************************" << std::endl;
    std::cout << "**** testing threaded middle-size set ****" << std::endl;
    std::cout << "******************************************" << std::endl;

    std::vector<u32> keys(128);
    std::iota(keys.begin(), keys.end(), 0);

    std::random_device device;
    //std::minstd_rand gen(12345);
    std::minstd_rand gen(device());

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


void test_threaded_stress()
{
    std::cout << "*********************************************" << std::endl;
    std::cout << "**** testing threaded elements insertion ****" << std::endl;
    std::cout << "*********************************************" << std::endl;

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


void test_threaded_insert_equal()
{
    std::cout << "***************************************************" << std::endl;
    std::cout << "**** testing threaded equal elements insertion ****" << std::endl;
    std::cout << "***************************************************" << std::endl;

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

void test_threaded_insert_delete()
{
    std::cout << "****************************************" << std::endl;
    std::cout << "**** testing threaded insert-delete ****" << std::endl;
    std::cout << "****************************************" << std::endl;

    auto count = 128;

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

    for (i32 k = 0; k < 100'000; k++)
    {
        if (k % 1000 == 0)
        {
            std::cout << "test: " << k << std::endl;
        }

        shuffle(keys, gen);

        for (i32 c = 0 ; c < (i32)keys.size() / 2; c++)
        {
            ms.add(keys[c]);

            ms.traverse();
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

            ms.traverse();
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

            ms.traverse();
            assert(ms.invariant());
            assert(ms.validNil());
        };

        for (i32 m = 0; m < 2'000; m++)
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


void test_threaded_iteration()
{
    std::cout << "************************************" << std::endl;
    std::cout << "**** testing threaded iteration ****" << std::endl;
    std::cout << "************************************" << std::endl;

    std::vector<u32> keys{1,2,3,4,5,6};

    MySet ms;
    for (u32 k = 0; k < 3; k++)
    {
        for(auto& key : keys)
        {
            ms.add(key);

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
        }
    }

    std::cout << "testing ended" << std::endl << std::endl;
}


void test_threaded_interactive()
{
    std::cout << "**************************************" << std::endl;
    std::cout << "**** interactive testing threaded ****" << std::endl;
    std::cout << "**************************************" << std::endl;

    // h <key> - has <key>
    // r <key> - remove <key>
    // a <key> - add <key>
    // lb <key> - lower bound of <key>
    // ub <key> - upper bound of <key>
    // if - iterate forward
    // ib - iterate backward
    // dfs - print debug info
    // e - exit

    MySet ms;
    std::string command;
    u32 key{};

    std::cout << ">:";
    while (std::cin >> command)
    {
        if (command == "h")
        {
            std::cin >> key;

            std::cout << ms.has(key) << std::endl;
        }
        else if (command == "r")
        {
            std::cin >> key;

            ms.remove(key);
        }
        else if (command == "a")
        {
            std::cin >> key;

            ms.add(key);
        }
        else if (command == "lb")
        {
            std::cin >> key;

            auto lb = ms.lowerBound(key);
            if (!ms.isNil(lb))
            {
                std::cout << lb->key << std::endl;
            }
            else
            {
                std::cout << "nil" << std::endl;
            }   
        }
        else if (command == "ub")
        {
            std::cin >> key;

            auto ub = ms.upperBound(key);
            if (!ms.isNil(ub))
            {
                std::cout << ub->key << std::endl;
            }
            else
            {
                std::cout << "nil" << std::endl;
            }
        }
        else if (command == "if")
        {
            std::cout << "Forward: ";
            auto it  = ms.min();
            auto nil = ms.nil.get();
            while (it != nil)
            {
                std::cout << it->key << " ";

                it = ms.succ(it);
            }
            std::cout << std::endl;
        }
        else if (command == "ib")
        {
            std::cout << "Backward: ";
            auto it  = ms.max();
            auto nil = ms.nil.get();
            while (it != nil)
            {
                std::cout << it->key << " ";

                it = ms.pred(it);
            }
            std::cout << std::endl;
        }
        else if (command == "dfs")
        {
            ms.dfs();
        }
        else if (command == "e")
        {
            break;
        }
        else
        {
            std::cout << "invalid command" << std::endl;
        }

        std::cout << ">:";
    }
}



namespace
{
    void test_threaded_lower_bound(const std::vector<u32>& keys, u32 left, u32 right)
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

    void test_threaded_upper_bound(const std::vector<u32>& keys, u32 left, u32 right)
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

void test_threaded_lower_bound()
{
    std::cout << "*****************************" << std::endl;
    std::cout << "**** testing lower_bound ****" << std::endl;
    std::cout << "*****************************" << std::endl;

    test_threaded_lower_bound({1,2,3,4,5,6,7,8,9,10}, 0, 11);
    test_threaded_lower_bound({2,4,6,8,10}, 0, 11);
    test_threaded_lower_bound({1,3,5,7,9}, 0, 11);
    test_threaded_lower_bound({1,3,5,5,5,5,7,7,7,7,9}, 0, 11);

    std::cout << "testing finished" << std::endl << std::endl;
}

void test_threaded_upper_bound()
{
    std::cout << "*****************************" << std::endl;
    std::cout << "**** testing upper_bound ****" << std::endl;
    std::cout << "*****************************" << std::endl;

    test_threaded_upper_bound({1,2,3,4,5,6,7,8,9,10}, 0, 11);
    test_threaded_upper_bound({2,4,6,8,10}, 0, 11);
    test_threaded_upper_bound({1,3,5,7,9}, 0, 11);
    test_threaded_upper_bound({1,3,5,5,5,5,7,7,7,7,9}, 0, 11);

    std::cout << "testing finished" << std::endl << std::endl;
}