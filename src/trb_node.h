#pragma once

#include <type_traits>

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


    // TODO : add specialization that take into consideration constructability of a Key
    template<class Traits, class = void>
    struct TreeNode;

    template<class Traits>
    struct TreeNode<Traits, std::enable_if_t<Traits::threaded>>
    {
        static constexpr bool threaded = true;

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
        static constexpr bool threaded = false;

        using Key = typename Traits::Key;

        Key key{};

        TreeNode* parent{};
        TreeNode* left{};
        TreeNode* right{};

        Color color{Color::Black};
    };
}
