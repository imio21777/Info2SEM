#pragma once

#include <cstddef>
#include <iterator>

namespace lab {

struct forward_iterator_tag {};
struct bidirectional_iterator_tag: forward_iterator_tag {};
struct random_access_iterator_tag : bidirectional_iterator_tag {};

template<class I>
struct iterator_traits {
    using iterator_category = typename I::iterator_category;
    using difference_type = typename I::difference_type;
    using value_type = typename I::value_type;
    using ptr = typename I::ptr;
    using ref = typename I::ref;
};

// template<class T>
// struct iterator_traits<T*> {
//     using iterator_category = random_access_iterator_tag;
//     using difference_type = std::ptrdiff_t;
//     using value_type = T;
//     using ptr = I*;
//     using ref = I&;
// };

// template<class T>
// struct iterator_traits<const T*> {
//     using iterator_category = random_access_iterator_tag;
//     using difference_type = std::ptrdiff_t;
//     using value_type = T;
//     using ptr = I*;
//     using ref = I&;
// };

}
