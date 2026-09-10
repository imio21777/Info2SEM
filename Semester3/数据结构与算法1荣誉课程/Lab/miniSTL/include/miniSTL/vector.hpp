#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <limits>
#include <stdexcept>
#include <utility>
#include <compare>
#include <initializer_list>

template <class T, class Alloc = std::allocator<T>>
struct Vector
{
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = T &;
    using const_reference = T const &;
    using iterator = T *;
    using const_iterator = T const *;
    using reverse_iterator = std::reverse_iterator<T *>;
    using const_reverse_iterator = std::reverse_iterator<T const *>;

    T *m_data;
    size_t m_size;
    size_t m_cap;
    [[no_unique_address]] Alloc m_alloc;

    Vector()
    {
        m_data = nullptr; // iterator
        m_size = 0;       // vector.size()
        m_cap = 0;        // vector.capacity()
    }

    // initializer_list constructor
    Vector(std::initializer_list<T> ilist, Alloc const &alloc = Alloc()) : Vector(ilist.begin(), ilist.end(), alloc)
    {
    }

    explicit Vector(size_t n, Alloc const &alloc = Alloc()) : m_alloc(alloc)
    {
        m_data = m_alloc.allocate(n);
        m_cap = n;
        m_size = n;
        for (auto i = 0; i != n; ++i)
        {
            std::construct_at(&m_data[i], T()); // use default data of T to initialize
        }
    }

    Vector(size_t n, T const &val, Alloc const &alloc = Alloc())
    {
        m_data = m_alloc.allocate(n);
        m_cap = n;
        m_size = n;
        for (auto i = 0; i != n; ++i)
        {
            std::construct_at(&m_data[i], val);
        }
    }

    // use iterator to initialize
    template <std::random_access_iterator InputIt>
    Vector(InputIt first, InputIt last, Alloc const &alloc = Alloc()) : m_alloc(alloc)
    {
        m_size = last - first;
        m_cap = m_size;
        m_data = m_alloc.allocate(m_size);
        for (auto i = 0; i < m_size; ++i)
        {
            std::construct_at(&m_data[i], *first);
            ++first;
        }
    }

    void clear()
    {
        for (auto i = 0; i < m_size; ++i)
        {
            std::destroy_at(&m_data[i]);
        }
        // clear will not change capacity
        m_size = 0;
    }

    void resize(size_t n)
    {
        if (n == m_size)
            return;
        if (n < m_size)
        {
            for (auto i = n; i < m_size; ++i)
            {
                std::destroy_at(&m_data[i]);
            }
            m_size = n;
        }
        else
        {
            // cannot use m_cap = n, you need to maintain 2*space for vectors to expand capacity
            reserve(n);
            for (auto i = m_size; i < n; ++i)
            {
                std::construct_at(&m_data[i], T());
            }
            m_size = n;
        }
    }

    void resize(size_t n, T const &val)
    {
        if (n == m_size)
            return;
        if (n < m_size)
        {
            for (auto i = n; i < m_size; ++i)
            {
                std::destroy_at(&m_data[i]);
            }
            m_size = n;
        }
        else
        {
            // cannot use m_cap = n, you need to maintain 2*space for vectors to expand capacity
            reserve(n);
            for (size_t i = m_size; i < n; ++i)
            {
                std::construct_at(&m_data[i], val);
            }
            m_size = n;
        }
    }

    void shrink_to_fit()
    {
        auto old_data = m_data;
        auto old_cap = m_cap;
        m_cap = m_size;

        if (!m_size)
        {
            m_data = nullptr;
        }
        else
        {
            // allocate memory
            m_data = m_alloc.allocate(m_size);
        }

        if (old_cap)
        {
            // reconstruct
            for (auto i = 0; i < m_size; ++i)
            {
                std::construct_at(&m_data[i], std::move(old_data[i]));
                std::destroy_at(&old_data[i]);
            }
            m_alloc.deallocate(old_data, old_cap);
        }
    }

    void reserve(size_t n)
    {
        if (n <= m_size)
            return;

        // expand to 2*m_cap or more
        n = n >= (m_cap * 2) ? n : (m_cap * 2);
        auto old_data = m_data;
        const auto old_cap = m_cap;

        m_cap = n;
        m_data = m_alloc.allocate(n);

        if (old_cap)
        {
            for (auto i = 0; i < m_size; ++i)
            {
                std::construct_at(&m_data[i], std::move(old_data[i]));
                std::destroy_at(&old_data[i]);
            }
            // for (auto i=0;i< m_size;i++) {
            //     std::destroy_at(&old_data[i]);
            // }
            m_alloc.deallocate(old_data, old_cap);
        }
    }

    [[nodiscard]] size_t capacity() const noexcept
    {
        return m_cap;
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_size == 0;
        // below is wrong
        // return m_data == nullptr;
    }

    T const &operator[](size_t i) const noexcept
    {
        return m_data[i];
    }

    T &operator[](size_t i) noexcept
    {
        // T temp;
        // for (auto j = 0; j < m_size; ++j) {
        //     temp = m_data[j];
        // }
        return m_data[i];
    }

    T const &at(size_t i) const
    {
        if (m_size <= i)
            throw std::out_of_range("vector::at out of range");
        return m_data[i];
    }

    T &at(size_t i)
    {
        if (m_size <= i)
            throw std::out_of_range("vector::at out of range");
        return m_data[i];
    }

    // move constructor
    Vector(Vector &&that) noexcept : m_data(that.m_data), m_size(that.m_size), m_cap(that.m_cap),
                                     m_alloc(std::move(that.m_alloc))
    {
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
    }

    Vector(Vector &&that, Alloc const &alloc) noexcept : m_data(that.m_data), m_size(that.m_size), m_cap(that.m_cap),
                                                         m_alloc(alloc)
    {
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
    }

    Vector &operator=(Vector &&that) noexcept
    {
        if (&that == this)
            return *this;

        // clear current vector
        for (auto i = 0; i < m_size; ++i)
        {
            std::destroy_at(&m_data[i]);
        }
        if (!m_cap)
        {
            m_alloc.deallocate(m_data, m_cap);
        }

        // move right value and return
        m_data = that.m_data;
        m_size = that.m_size;
        m_cap = that.m_cap;
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
        return *this;
    }

    void swap(Vector &that) noexcept
    {
        std::swap(m_data, that.m_data);
        std::swap(m_size, that.m_size);
        std::swap(m_cap, that.m_cap);
        std::swap(m_alloc, that.m_alloc);
    }

    // copy constructor
    Vector(Vector const &that)
    {
        m_alloc = that.m_alloc;
        m_size = that.m_size;
        m_cap = that.m_cap;
        if (m_size)
        {
            m_data = m_alloc.allocate(m_size);
            for (auto i = 0; i < m_size; ++i)
            {
                std::construct_at(&m_data[i], that.m_data[i]);
            }
        }
        else
        {
            m_data = nullptr;
        }
    }

    Vector(Vector const &that, Alloc const &alloc) : m_alloc(alloc)
    {
        m_size = that.m_size;
        m_cap = that.m_cap;
        if (m_size)
        {
            m_data = m_alloc.allocate(m_size);
            for (auto i = 0; i < m_size; ++i)
            {
                std::construct_at(&m_data[i], that.m_data[i]);
            }
        }
        else
        {
            m_data = nullptr;
        }
    }

    Vector &operator=(Vector const &that) noexcept
    {
        if (&that == this)
            return *this;

        reserve(that.m_size);
        m_size = that.m_size;
        for (size_t i = 0; i != m_size; i++)
        {
            std::construct_at(&m_data[i], that.m_data[i]);
        }
        return *this;
    }

    T const &front() const noexcept
    {
        return *m_data;
    }

    T &front() noexcept
    {
        return *m_data;
    }

    T const &back() const noexcept
    {
        return m_data[m_size - 1];
    }

    T &back() noexcept
    {
        // T temp;
        // for (auto j = 0; j < m_size; ++j) {
        //     temp = m_data[j];
        // }

        return m_data[m_size - 1];
    }

    void push_back(T const &val)
    {
        if (m_size + 1 >= m_cap)
            reserve(m_size + 1);
        std::construct_at(&m_data[m_size], val);
        ++m_size;
    }

    void push_back(T &&val)
    {
        if (m_size + 1 >= m_cap)
        {
            reserve(m_size + 1);
        }
        // T temp;
        // for (auto i = 0; i < m_size; ++i) {
        //     temp = m_data[i];
        // }
        std::construct_at(&m_data[m_size], std::move(val));
        ++m_size;
        // for (auto i = 0; i < m_size; ++i) {
        //     temp = m_data[i];
        // }
    }

    T *data() noexcept
    {
        return m_data;
    }

    T const *data() const noexcept
    {
        return m_data;
    }

    T const *cdata() const noexcept
    {
        return m_data;
    }

    T *begin() noexcept
    {
        return m_data;
    }

    T *end() noexcept
    {
        return m_data + m_size;
    }

    T const *begin() const noexcept
    {
        return m_data;
    }

    T const *end() const noexcept
    {
        return m_data + m_size;
    }

    T const *cbegin() const noexcept
    {
        return m_data;
    }

    T const *cend() const noexcept
    {
        return m_data + m_size;
    }

    std::reverse_iterator<T *> rbegin() noexcept
    {
        return std::make_reverse_iterator(m_data + m_size);
    }

    std::reverse_iterator<T *> rend() noexcept
    {
        return std::make_reverse_iterator(m_data);
    }

    std::reverse_iterator<T const *> crbegin() const noexcept
    {
        return std::make_reverse_iterator(m_data + m_size);
    }

    std::reverse_iterator<T const *> crend() const noexcept
    {
        return std::make_reverse_iterator(m_data);
    }

    void pop_back() noexcept
    {
        --m_size;
        std::destroy_at(&m_data[m_size]);
    }

    T *erase(T const *it) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        auto idx = it - m_data;
        --m_size;
        for (auto i = idx; i < m_size; ++i)
        {
            m_data[i] = std::move(m_data[i + 1]);
        }
        std::destroy_at(&m_data[m_size]);
        return const_cast<T *>(it);
    }

    T *erase(T const *first, T const *last) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        auto start = first - m_data;
        auto end = last - m_data;

        auto num = last - first;

        for (auto i = start; end < m_size; ++end)
        {
            m_data[i] = std::move(m_data[end]);
            ++i;
        }

        for (auto clear_start = m_size - num; clear_start < m_size; ++clear_start)
        {
            std::destroy_at(&m_data[clear_start]);
        }

        return const_cast<T *>(first);
    }

    void assign(size_t n, T const &val)
    {
        clear();
        reserve(n);
        m_size = n;
        for (auto i = 0; i < n; ++i)
        {
            std::construct_at(&m_data[i], val);
        }
    }

    template <std::random_access_iterator InputIt>
    void assign(InputIt first, InputIt last)
    {
        clear();
        const auto n = last - first;
        reserve(n);
        m_size = n;
        for (auto i = 0; i < n; ++i)
        {
            std::construct_at(&m_data[i], *first);
            ++first;
        }
    }

    void assign(std::initializer_list<T> ilist)
    {
        assign(ilist.begin(), ilist.end());
    }

    Vector &operator=(std::initializer_list<T> ilist)
    {
        assign(ilist.begin(), ilist.end());
        return *this;
    }

    T *insert(T const *it, T &&val)
    {
        auto idx = it - m_data;
        if (m_size + 1 >= m_cap)
        {
            reserve(m_size + 1);
        }
        // j ~ m_size => j + 1 ~ m_size + 1
        for (auto i = m_size; i > idx; --i)
        {
            // i is not contructed and when you are at i-1, you have already destroyed i
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size++;
        std::construct_at(&m_data[idx], std::move(val));
        return m_data + idx;
    }

    T *insert(T const *it, T const &val)
    {
        auto idx = it - m_data;
        if (m_size + 1 >= m_cap)
        {
            reserve(m_size + 1);
        }
        // j ~ m_size => j + 1 ~ m_size + 1
        for (auto i = m_size; i > idx; --i)
        {
            // i is not contructed and when you are at i-1, you have already destroyed i
            std::construct_at(&m_data[i], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size++;
        std::construct_at(&m_data[idx], val);
        return m_data + idx;
    }

    T *insert(T const *it, size_t n, T const &val)
    {
        auto idx = it - m_data;
        if (!n)
            return const_cast<T *>(it);
        if (m_size + n >= m_cap)
        {
            reserve(m_size + n);
        }

        for (auto i = m_size; i > idx; --i)
        {
            // copy from back
            std::construct_at(&m_data[i + n - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }

        m_size += n;

        for (auto i = idx; i < idx + n; ++i)
        {
            std::construct_at(&m_data[i], val);
        }

        return m_data + idx;
    }

    template <std::random_access_iterator InputIt>
    T *insert(T const *it, InputIt first, InputIt last)
    {
        auto idx = it - m_data;
        auto num = last - first;
        if (!num)
            return const_cast<T *>(it);

        if (m_size + num >= m_cap)
        {
            reserve(m_size + num);
        }

        for (auto i = m_size; i > idx; --i)
        {
            std::construct_at(&m_data[i + num - 1], std::move(m_data[i - 1]));
            std::destroy_at(&m_data[i - 1]);
        }
        m_size += num;

        for (auto i = idx; i < idx + num; ++i)
        {
            std::construct_at(&m_data[i], *first);
            ++first;
        }

        return m_data + idx;
    }

    T *insert(T const *it, std::initializer_list<T> ilist)
    {
        return insert(it, ilist.begin(), ilist.end());
    }

    ~Vector()
    {
        for (auto i = 0; i != m_size; ++i)
        {
            std::destroy_at(&m_data[i]);
        }
        if (m_cap)
        {
            m_alloc.deallocate(m_data, m_cap);
        }
    }

    bool operator==(Vector const &that) const noexcept
    {
        if (that.m_size != m_size)
        {
            return false;
        }

        for (auto i = 0; i < m_size; ++i)
        {
            if (that.m_data[i] != m_data[i])
            {
                return false;
            }
        }

        return true;
    }
};
