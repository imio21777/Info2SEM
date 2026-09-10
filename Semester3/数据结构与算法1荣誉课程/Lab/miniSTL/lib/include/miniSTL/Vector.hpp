#pragma once

#include <memory>
#include <cstddef>
#include <miniSTL/Iterator.hpp>

namespace lab {

template<class T>
struct _Vector_iterator {
public:
    using iterator_category = random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using ptr = T*;
    using ref = T&;

private:
    ptr data;

};


template<class T, class Alloc = std::allocator<T>>
struct Vector {

public:
    using value_type = T;
    using size_type = size_t;
    using ptr = value_type*;
    using const_ptr = const ptr;
    using ref = value_type&;
    using const_ref = const ref;
    using iter = _Vector_iterator<T>;

private:
    ptr _start;
    ptr _finish;
    ptr _end_of_capacity;

    static constexpr size_type default_capacity = 8;
    static constexpr float expand_ratio = 2.0f;
    static constexpr float shrink_ratio = 0.5f;
    static constexpr float shrink_threshold = 0.25f;

// private:
//     void _expand();
//     void _shrink();

// public:
//     Vector();
//     ~Vector();
//     Vector(const Vector<T, Alloc>&);
//     Vector(const Vector<T, Alloc>&&);

//     Vector<T, Alloc>& operator=(const Vector<T, Alloc>&);
//     Vector<T, Alloc>& operator=(const Vector<T, Alloc>&&);

//     Vector(size_type n, T default_value = T());
//     Vector(std::initializer_list<T> values);

//     T& operator[](const size_t index);

//     size_type size() const;
//     size_type capacity() const;

//     ref front() const;
//     ref back() const;

//     iter begin() const;
//     iter end() const;

//     void push_back();
//     void pop_back();
//     void clear();
};


}
