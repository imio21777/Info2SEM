#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <miniSTL/stl.hpp>
#include <vector>
#include <list>

struct M_int {
    int m_value;

    M_int() : m_value(0) {
    };

    M_int(int v) : m_value(v) {
    }

    auto operator==(M_int const &that) noexcept {
        return m_value == that.m_value;
    }
};

TEST_CASE("test vector", "[vector]") {
    std::vector<M_int> tmp_vec({0, 1, 2});

    SECTION("test constructor") {
        {
            Vector<M_int> vec;
        } {
            Vector<M_int> vec({0, 1, 2});
            for (int i = 0; i < 3; i++)
                REQUIRE(vec[i].m_value == i);
        } {
            Vector<M_int> vec(3, 0);
            for (int i = 0; i < 3; i++)
                REQUIRE(vec[i].m_value == 0);
        } {
            Vector<M_int> vec(tmp_vec.begin(), tmp_vec.end());
            for (int i = 0; i < 3; i++)
                REQUIRE(vec[i].m_value == i);
        }
    };

    SECTION("test size() capacity() shrink_to_fit() resize() reserve() assign()") {
        Vector<M_int> m_vec;
        REQUIRE(m_vec.size() == 0);
        REQUIRE(m_vec.empty());
        m_vec.resize(10);
        REQUIRE(m_vec.size() == 10);
        REQUIRE(m_vec.capacity() >= 10);
        m_vec.shrink_to_fit();
        REQUIRE(m_vec.size() == m_vec.capacity());
        m_vec.reserve(100);
        REQUIRE(m_vec.capacity() == 100);
        m_vec.assign(3, 10);
        for (int i = 0; i < 3; i++)
            REQUIRE(m_vec[i].m_value == 10);
    };

    SECTION("test operator[] at()") {
        Vector<M_int> m_vec({0, 1, 2, 3, 4, 5});
        for (int i = 0; i < 5; i++) {
            REQUIRE(m_vec[i].m_value == i);
            REQUIRE(m_vec.at(i).m_value == i);
        }
        for (int i = 0; i < 5; i++)
            m_vec[i].m_value = 0;
        for (int i = 0; i < 5; i++) {
            REQUIRE(m_vec[i].m_value == 0);
            REQUIRE(m_vec.at(i).m_value == 0);
        }
    }

    SECTION("test push_back() pop_back() front() back() erase() insert()") {
        Vector<M_int> m_vec({0, 1, 2, 3, 4, 5});
        // M_int temp;
        // for (auto i: m_vec) {
        //     temp = i;
        // }
        //
        // for (auto i = 0; i < m_vec.m_size; ++i) {
        //     temp = m_vec[i];
        // }

        m_vec.push_back(6);
        REQUIRE(m_vec.back().m_value == 6);
        m_vec.pop_back();
        REQUIRE(m_vec.back().m_value == 5);

        REQUIRE(m_vec.front().m_value == 0);
        m_vec.erase(m_vec.begin());
        REQUIRE(m_vec.front().m_value == 1);

        m_vec.insert(m_vec.begin() + 3, -1);
        REQUIRE(m_vec[3].m_value == -1);
    }

    SECTION("test begin() end() rbegin() rend()") {
        Vector<M_int> m_vec({0, 1, 2, 3, 4, 5});
        for (auto it = m_vec.begin(); it != m_vec.end(); it++) {
            REQUIRE(it->m_value == it - m_vec.begin());
        }
        auto rend = m_vec.rend();
        for (auto rit = m_vec.rbegin(); rit != rend; rit++) {
            REQUIRE((*rit).m_value == 5 - (rit - m_vec.rbegin()));
        }
    }

    SECTION("test operator==") {
        Vector<M_int> a({0, 1, 2, 3, 4, 5});
        Vector<M_int> b({0, 1, 2, 3, 4, 6});
        REQUIRE(a == a);
        REQUIRE_FALSE(a == b);
    }
}
