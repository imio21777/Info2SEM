#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <miniSTL/stl.hpp>
#include <vector>
#include <list>

struct M_int {
    int m_value;

    M_int() : m_value(0) {};
    M_int(int v) : m_value(v) {}

    auto operator==(M_int const &that) noexcept {
        return m_value == that.m_value;
    }
};


TEST_CASE("test list", "[list]") {
    std::list<M_int> tmp_list({0, 1, 2});

    SECTION("test constructor") {
        {
            List<M_int> lst;
        }
        {
            List<M_int> lst({0, 1, 2});
            auto it = lst.begin();
            for (int i = 0; i < 3; i++) {
                REQUIRE((*it).m_value == i);
                ++it;
            }
        }
        {
            List<M_int> lst(3, 0);
            for (auto item : lst)
                REQUIRE(item.m_value == 0);
        }
        {
            List<M_int> lst(tmp_list.begin(), tmp_list.end());
            auto it = lst.begin();
            for (int i = 0; i < 3; i++) {
                REQUIRE((*it).m_value == i);
                ++it;
            }
        }
    };

    SECTION("test size() assign()") {
        List<M_int> lst;
        REQUIRE(lst.size() == 0);
        REQUIRE(lst.empty());
        lst.assign(3, 10);
        for (auto item : lst)
            REQUIRE(item.m_value == 10);
        lst.assign({0, 1, 2});
        auto it = lst.begin();
        for (int i = 0; i < 3; i++) {
            REQUIRE((*it).m_value == i);
            ++it;
        }
    };

    SECTION("test push_front() push_back() erase() insert()") {
        List<M_int> lst({0, 1, 2, 3, 4, 5});

        REQUIRE(lst.front().m_value == 0);
        lst.erase(lst.begin());
        REQUIRE(lst.front().m_value == 1);

        lst.push_front(-1);
        REQUIRE(lst.front().m_value == -1);

        lst.push_back(100);
        REQUIRE(lst.back().m_value == 100);

        lst.insert(lst.begin(), -100);
        REQUIRE((*lst.begin()).m_value == -100);
    }

    SECTION("test begin() end() rbegin() rend()") {
        List<M_int> lst({0, 1, 2, 3, 4, 5});
        int i;
        i = 0;
        for (auto it = lst.begin(); it != lst.end(); it++) {
            REQUIRE((*it).m_value == i);
            i++;
        }
        i = 5;
        auto rend = lst.rend();
        for (auto rit = lst.rbegin(); rit != rend; rit++) {
            REQUIRE((*rit).m_value == i);
            i--;
        }
    }

    SECTION("test operator==") {
        List<M_int> a({0, 1, 2, 3, 4, 5});
        List<M_int> b({0, 1, 2, 3, 4, 6});
        REQUIRE(a == a);
        REQUIRE_FALSE(a == b);
    }
}
