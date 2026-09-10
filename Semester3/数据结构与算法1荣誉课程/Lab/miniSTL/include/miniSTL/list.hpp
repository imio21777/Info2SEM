#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <limits>
#include <stdexcept>
#include <utility>
#include <compare>
#include <initializer_list>

template <class T>
struct ListBaseNode
{
    ListBaseNode *m_next;
    ListBaseNode *m_prev;

    inline T &value();

    inline T const &value() const;
};

template <class T>
struct ListValueNode : ListBaseNode<T>
{
    union
    {
        T m_value;
    };
};

template <class T>
inline T &ListBaseNode<T>::value()
{
    return static_cast<ListValueNode<T> &>(*this).m_value;
}

template <class T>
inline T const &ListBaseNode<T>::value() const
{
    return static_cast<ListValueNode<T> const &>(*this).m_value;
}

template <class T, class Alloc = std::allocator<T>>
struct List
{
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = T &;
    using const_reference = T const &;

private:
    using ListNode = ListBaseNode<T>;

    // alloc space for nodes
    using AllocNode = std::allocator_traits<Alloc>::template rebind_alloc<ListValueNode<T>>;

    ListNode m_dummy;
    size_t m_size;
    [[no_unique_address]] Alloc m_alloc;

public:
    List()
    {
        m_size = 0;
        m_dummy.m_prev = &m_dummy;
        m_dummy.m_next = &m_dummy;
    }

    List(List &&that) noexcept : m_size(std::move(that.m_size)), m_alloc(std::move(that.m_alloc))
    {
        // get situation of m_dummy
        auto prev_node = that.m_dummy.m_prev;
        auto next_node = that.m_dummy.m_next;
        prev_node->m_next = &m_dummy;
        next_node->m_prev = &m_dummy;
        // copy head node
        m_dummy = that.m_dummy;

        // reset that.m_dummy
        that.m_dummy.m_prev = &that.m_dummy;
        that.m_dummy.m_next = &that.m_dummy;
        that.m_size = 0;
    }

    // cannot use std::move for const
    List(List const &that) noexcept : m_size(that.m_size), m_alloc(that.m_alloc)
    {
        ListNode *prev = &m_dummy;
        auto first = that.cbegin();

        while (first != that.cend())
        {
            ListNode *node = AllocNode{m_alloc}.allocate(1);
            prev->m_next = node;
            node->m_prev = prev;
            std::construct_at(&node->value(), *first);
            prev = node;
            ++first;
        }
        m_dummy.m_prev = prev;
        prev->m_next = &m_dummy;
    }

    List &operator=(List const &that)
    {
        // use const iterator
        assign(that.cbegin(), that.cend());
        return *this;
    }

    bool empty() noexcept
    {
        return m_dummy.m_prev = m_dummy.m_next;
    }

    T &front() noexcept
    {
        return m_dummy.m_next->value();
    }

    T &back() noexcept
    {
        return m_dummy.m_prev->value();
    }

    T const &front() const noexcept
    {
        return m_dummy.m_prev->value();
    }

    T const &back() const noexcept
    {
        return m_dummy.m_prev->value();
    }

    explicit List(size_t n, Alloc const &alloc = Alloc()) : m_size(n), m_alloc(alloc)
    {
        ListNode *prev = &m_dummy;
        for (size_t i = n; i > 0; --i)
        {
            ListNode *newNode = AllocNode{m_alloc}.allocate(1);
            prev->m_next = newNode;
            newNode->m_prev = prev;

            std::construct_at(&newNode->value(), T());

            prev = newNode;
        }
        m_dummy.m_prev = prev;
        prev->m_next = &m_dummy;
    }

    List(size_t n, T const &val, Alloc const &alloc = Alloc()) : m_size(n), m_alloc(alloc)
    {
        ListNode *prev = &m_dummy;
        for (size_t i = n; i > 0; --i)
        {
            ListNode *newNode = AllocNode{m_alloc}.allocate(1);
            prev->m_next = newNode;
            newNode->m_prev = prev;

            std::construct_at(&newNode->value(), val);

            prev = newNode;
        }
        m_dummy.m_prev = prev;
        prev->m_next = &m_dummy;
    }

    template <std::input_iterator InputIt>
    List(InputIt first, InputIt last, Alloc const &alloc = Alloc()) : m_alloc(alloc)
    {
        m_size = 0;
        ListNode *prev = &m_dummy;
        while (first != last)
        {
            ListNode *newNode = AllocNode{m_alloc}.allocate(1);
            prev->m_next = newNode;
            newNode->m_prev = prev;

            std::construct_at(&newNode->value(), *first);
            prev = newNode;
            ++first;
            ++m_size;
        }
        m_dummy.m_prev = prev;
        prev->m_next = &m_dummy;
    }

    List(std::initializer_list<T> ilist, Alloc const &alloc = Alloc()) : List(ilist.begin(), ilist.end(), alloc)
    {
    }

    List &operator=(std::initializer_list<T> ilist)
    {
        assign(ilist);
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept { return m_size; }

    template <std::input_iterator InputIt>
    void assign(InputIt first, InputIt last)
    {
        clear();

        m_size = 0;
        ListNode *prev = &m_dummy;
        while (first != last)
        {
            ListNode *newNode = AllocNode{m_alloc}.allocate(1);
            prev->m_next = newNode;
            newNode->m_prev = prev;

            std::construct_at(&newNode->value(), *first);
            prev = newNode;
            ++first;
            ++m_size;
        }
        m_dummy.m_prev = prev;
        prev->m_next = &m_dummy;
    }

    void assign(std::initializer_list<T> ilist)
    {
        assign(ilist.begin(), ilist.end());
        // clear();
        //
        // m_size = 0;
        // ListNode *prev = &m_dummy;
        // while (ilist.begin() != ilist.end()) {
        //     ListNode *newNode = AllocNode{m_alloc}.allocate(1);
        //     prev->m_next = newNode;
        //     newNode->m_prev = prev;
        //
        //     std::construct_at(&newNode->value(), *ilist.begin());
        //     prev = newNode;
        //     ++ilist.begin();
        //     ++m_size;
        // }
        // m_dummy.m_prev = prev;
        // prev->m_next = &m_dummy;
    }

    void assign(size_t n, T const &val)
    {
        clear();

        m_size = n;
        ListNode *prev = &m_dummy;
        for (size_t i = n; i > 0; --i)
        {
            ListNode *newNode = AllocNode{m_alloc}.allocate(1);
            prev->m_next = newNode;
            newNode->m_prev = prev;

            std::construct_at(&newNode->value(), val);

            prev = newNode;
        }
        m_dummy.m_prev = prev;
        prev->m_next = &m_dummy;
    }

    void push_back(T const &val)
    {
        ++m_size;
        ListNode *newNode = AllocNode{m_alloc}.allocate(1);
        newNode->m_prev = m_dummy.m_prev;
        newNode->m_next = &m_dummy;
        m_dummy.m_prev->m_next = newNode;
        m_dummy.m_prev = newNode;
        std::construct_at(&newNode->value(), val);
    }

    void push_back(T &&val)
    {
        ++m_size;
        ListNode *newNode = AllocNode{m_alloc}.allocate(1);
        newNode->m_prev = m_dummy.m_prev;
        newNode->m_next = &m_dummy;
        m_dummy.m_prev->m_next = newNode;
        m_dummy.m_prev = newNode;
        std::construct_at(&newNode->value(), std::move(val));
    }

    void push_front(T const &val)
    {
        ++m_size;
        auto newNode = AllocNode{m_alloc}.allocate(1);

        m_dummy.m_next->m_prev = newNode;
        newNode->m_next = m_dummy.m_next;
        newNode->m_prev - &m_dummy;
        m_dummy.m_next = newNode;
        std::construct_at(&newNode->value(), val);
    }

    void push_front(T &&val)
    {
        ++m_size;
        auto newNode = AllocNode{m_alloc}.allocate(1);

        m_dummy.m_next->m_prev = newNode;
        newNode->m_next = m_dummy.m_next;
        newNode->m_prev = &m_dummy;
        m_dummy.m_next = newNode;
        std::construct_at(&newNode->value(), std::move(val));
    }

    ~List()
    {
        clear();
    }

    void clear() noexcept
    {
        ListNode *current = m_dummy.m_next;
        while (current != &m_dummy)
        {
            auto temp = current->m_next;
            std::destroy_at(&current->value());

            AllocNode{m_alloc}.deallocate(static_cast<ListValueNode<T> *>(current), 1);

            current = temp;
        }
        m_dummy.m_next = &m_dummy;
        m_dummy.m_prev = &m_dummy;
        m_size = 0;
    }

    struct iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T *;
        using reference = T &;

    private:
        ListNode *m_curr;

        friend List;

        explicit iterator(ListNode *curr) : m_curr(curr)
        {
        }

    public:
        iterator() = default;

        iterator &operator++() noexcept
        {
            // overload ++iterator
            m_curr = m_curr->m_next;
            return *this;
        }

        iterator operator++(int) noexcept
        {
            // overload iterator++
            // return origin then increase it
            auto ret = *this;
            m_curr = m_curr->m_next;
            return ret;
        }

        iterator &operator--() noexcept
        {
            // overload --iterator
            m_curr = m_curr->m_prev;
            return *this;
        }

        iterator operator--(int) noexcept
        {
            // overload iterator--
            // return origin then decrease it
            auto ret = *this;
            m_curr = m_curr->m_prev;
            return ret;
        }

        T &operator*() const noexcept
        {
            return m_curr->value();
        }

        bool operator!=(iterator const &that) const noexcept
        {
            // should use == for iterator
            // should not directly compare pointers for better modifying code
            // which means do not use return !(m_curr == that.m_curr);
            return !(*this == that);
            // return !(m_curr == that.m_curr);
        }

        bool operator==(iterator const &that) const noexcept
        {
            return m_curr == that.m_curr;
        }
    };

    struct const_iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T const *;
        using reference = T const &;

    private:
        ListNode const *m_curr;

        friend List;

        explicit const_iterator(ListNode const *curr) noexcept : m_curr(curr)
        {
        }

    public:
        const_iterator() = default;

        const_iterator(iterator that) noexcept : m_curr(that.m_curr)
        {
        }

        explicit operator iterator() noexcept
        {
            return iterator{const_cast<ListNode *>(m_curr)};
        }

        const_iterator &operator++() noexcept
        {
            // overload ++iterator
            m_curr = m_curr->m_next;
            return *this;
        }

        const_iterator operator++(int)
        {
            // overload iterator++
            // return origin then increase it
            auto ret = *this;
            m_curr = m_curr->m_next;
            return ret;
        }

        const_iterator &operator--() noexcept
        {
            // overload --iterator
            m_curr = m_curr->m_prev;
            return *this;
        }

        const_iterator operator--(int) noexcept
        {
            // overload iterator--
            // return origin then decrease it
            auto ret = *this;
            m_curr = m_curr->m_prev;
            return ret;
        }

        T const &operator*() const noexcept
        {
            return m_curr->value();
        }

        bool operator!=(const_iterator const &that) const noexcept
        {
            return !(*this == that);
            // return !(m_curr == that.m_curr);
        }

        bool operator==(const_iterator const &that) const noexcept
        {
            return m_curr == that.m_curr;
        }
    };

    iterator begin() noexcept
    {
        return iterator{m_dummy.m_next};
    }

    iterator end() noexcept
    {
        return iterator{&m_dummy};
    }

    const_iterator cbegin() const noexcept
    {
        return const_iterator{m_dummy.m_next};
    }

    const_iterator cend() const noexcept
    {
        return const_iterator{&m_dummy};
    }

    const_iterator begin() const noexcept
    {
        return const_iterator{m_dummy.m_next};
    }

    const_iterator end() const noexcept
    {
        return const_iterator{&m_dummy};
    }

    using reverse_iterator = std::reverse_iterator<iterator>;
    using reverse_const_iterator = std::reverse_iterator<const_iterator>;

    reverse_iterator rbegin() noexcept
    {
        return std::make_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept
    {
        return std::make_reverse_iterator(begin());
    }

    reverse_const_iterator crbegin() const noexcept
    {
        return std::make_reverse_iterator(cend());
    }

    reverse_const_iterator crend() const noexcept
    {
        return std::make_reverse_iterator(cbegin());
    }

    reverse_const_iterator rbegin() const noexcept
    {
        return std::make_reverse_iterator(cend());
    }

    reverse_const_iterator rend() const noexcept
    {
        return std::make_reverse_iterator(cbegin());
    }

    iterator erase(const_iterator pos) noexcept
    {
        auto noConstNode = const_cast<ListNode *>(pos.m_curr);

        auto next = noConstNode->m_next;
        noConstNode->m_next->m_prev = noConstNode->m_prev;
        noConstNode->m_prev->m_next = noConstNode->m_next;
        std::destroy_at(&noConstNode->value());

        AllocNode{m_alloc}.deallocate(static_cast<ListValueNode<T> *>(noConstNode), 1);
        --m_size;
        return iterator{next};
    }

    iterator erase(const_iterator first, const_iterator last) noexcept
    {
        while (first != last)
        {
            first = erase(first);
        }
        return iterator(first);
    }

    void pop_front() noexcept
    {
        erase(this->begin());
    }

    void pop_back() noexcept
    {
        // erase(std::prev(end());
        erase(this->rbegin());
    }

    iterator insert(const_iterator pos, const T &val)
    {
        ++m_size;
        ListNode *curr = AllocNode{m_alloc}.allocate(1);
        auto *next = const_cast<ListNode *>(pos.m_curr);

        curr->m_prev = next->m_prev;
        next->m_prev->m_next = curr;
        curr->m_next = next;
        next->m_prev = curr;

        std::construct_at(&curr->value(), val);
        return iterator{curr};
    }

    iterator insert(const_iterator pos, T &&val)
    {
        ++m_size;
        ListNode *curr = AllocNode{m_alloc}.allocate(1);
        auto *next = const_cast<ListNode *>(pos.m_curr);

        curr->m_prev = next->m_prev;
        next->m_prev->m_next = curr;
        curr->m_next = next;
        next->m_prev = curr;

        std::construct_at(&curr->value(), std::move(val));
        return iterator{curr};
    }

    iterator insert(const_iterator pos, size_t n, T const &val)
    {
        m_size += n;
        auto result = pos;
        bool isFirstInsert = true;

        while (n)
        {
            if (isFirstInsert)
            {
                result = insert(pos, val);
                ++pos;
                isFirstInsert = false;
                ++m_size;
                --n;
                continue;
            }
            --n;
            ++m_size;
            insert(pos, val);
            ++pos;
        }

        return iterator{result};
    }

    template <std::input_iterator InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        auto result = pos;
        bool isFirstInsert = true;

        while (first != last)
        {
            if (isFirstInsert)
            {
                result = insert(pos, *first);
                ++pos;
                ++first;
                isFirstInsert = false;
                ++m_size;
                continue;
            }
            ++m_size;
            insert(pos, *first);
            ++pos;
            ++first;
        }
        return iterator{result};
    }

    iterator insert(const_iterator pos, std::initializer_list<T> ilist)
    {
        return insert(pos, ilist.begin(), ilist.end());
    }

    bool operator==(List const &that) const noexcept
    {
        if (that.size() != m_size)
            return false;
        const_iterator thisIterator = this->cbegin();
        const_iterator thatIterator = that.cbegin();
        const_iterator thisEnd = this->cend();
        const_iterator thatEnd = that.cend();

        while (thisIterator != thisEnd && thatIterator != thatEnd)
        {
            auto temp1 = *thisIterator;
            auto temp2 = *thatIterator;
            if (temp1 != temp2)
            {
                return false;
            }
            // if (*thisIterator != *thatIterator)
            //     return false;
            ++thisIterator;
            ++thatIterator;
        }

        return true;
    }

    // bool operator==(List const &that) const noexcept {
    //     // 首先检查两个列表的大小是否相等
    //     if (m_size != that.m_size) {
    //         return false;
    //     }
    //
    //     // 使用迭代器遍历两个列表，并比较每个位置的元素
    //     const_iterator it1(this->m_dummy.m_next);
    //     const_iterator it2(that.m_dummy.m_next);
    //     const_iterator end1(&this->m_dummy);
    //     const_iterator end2(&that.m_dummy);
    //     while (it1 != end1 && it2 != end2) {
    //         auto temp1 = *it1;
    //         auto temp2 = *it2;
    //
    //         if (!(temp1==temp2)) {
    //             return false;
    //         }
    //         ++it1;
    //         ++it2;
    //     }
    //
    //     // 所有元素都相等
    //     return true;
    // }
};
