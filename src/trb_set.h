#pragma once

#include "trb_tree.h"

namespace ds
{
    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = trb::DefaultAllocator>
    using Set = trb::Tree<trb::TreeTraits<key_t, compare_t, allocator_t, false, false>>;

    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = trb::DefaultAllocator>
    using Multiset = trb::Tree<trb::TreeTraits<key_t, compare_t, allocator_t, false, true>>;

    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = trb::DefaultAllocator>
    using ListSet = trb::Tree<trb::TreeTraits<key_t, compare_t, allocator_t, true, false>>;

    template<class key_t, class compare_t = std::less<key_t>, template<class> class allocator_t = trb::DefaultAllocator>
    using ListMultiset = trb::Tree<trb::TreeTraits<key_t, compare_t, allocator_t, true, true>>;
}
