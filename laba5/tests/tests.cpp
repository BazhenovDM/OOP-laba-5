#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include "../include/pmr_list.hpp"

using namespace std;

struct Employee {
    std::string name;
    int age;
    double salary;
    Employee() = default;
    Employee(std::string n, int a, double s) : name(std::move(n)), age(a), salary(s) {}
    bool operator==(Employee const& o) const {
        return name == o.name && age == o.age && std::abs(salary - o.salary) < 1e-9;
    }
};

static std::vector<int> collectInts(pmr_list<int>& lst) {
    std::vector<int> out;
    for (auto const& v : lst) out.push_back(v);
    return out;
}

static std::vector<Employee> collectEmployees(pmr_list<Employee>& lst) {
    std::vector<Employee> out;
    for (auto const& v : lst) out.push_back(v);
    return out;
}

TEST(Basic, EmptyListHasNoElements) {
    pmr_list<int> l;
    size_t cnt = 0;
    for ([[maybe_unused]] auto const& v : l) ++cnt;
    EXPECT_EQ(cnt, 0u);
}

TEST(Basic, SingleEmplaceBack) {
    pmr_list<int> l;
    l.emplace_back(42);
    auto v = collectInts(l);
    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 42);
}

TEST(Basic, ManyEmplaceBackPreserveOrder) {
    pmr_list<int> l;
    for (int i = 1; i <= 10; ++i) l.emplace_back(i * 10);
    auto v = collectInts(l);
    ASSERT_EQ(v.size(), 10u);
    for (size_t i = 0; i < 10; ++i) EXPECT_EQ(v[i], static_cast<int>((i + 1) * 10));
}

TEST(Basic, EmplaceFrontWorks) {
    pmr_list<int> l;
    l.emplace_front(1);
    l.emplace_front(2);
    l.emplace_front(3);
    auto v = collectInts(l);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 3);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 1);
}

TEST(Removal, PopBackRemovesLast) {
    pmr_list<int> l;
    for (int i = 1; i <= 5; ++i) l.emplace_back(i);
    l.pop_back();
    auto v = collectInts(l);
    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v.back(), 4);
}

TEST(Removal, PopFrontRemovesFirst) {
    pmr_list<int> l;
    for (int i = 1; i <= 5; ++i) l.emplace_back(i);
    l.pop_front();
    auto v = collectInts(l);
    ASSERT_EQ(v.size(), 4u);
    EXPECT_EQ(v.front(), 2);
}

TEST(Ordering, MixedFrontBackOperations) {
    pmr_list<int> l;
    l.emplace_back(10);
    l.emplace_front(5);
    l.emplace_back(20);
    l.emplace_front(2);
    auto v = collectInts(l);
    std::vector<int> expect = {2,5,10,20};
    EXPECT_EQ(v, expect);
}

TEST(ComplexType, EmplaceEmployeeAndCheckFields) {
    pmr_list<Employee> staff;
    staff.emplace_back(Employee("Alice", 30, 60000.0));
    staff.emplace_back(Employee("Bob", 28, 52000.0));
    auto v = collectEmployees(staff);
    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0].name, "Alice");
    EXPECT_EQ(v[0].age, 30);
    EXPECT_DOUBLE_EQ(v[0].salary, 60000.0);
    EXPECT_EQ(v[1].name, "Bob");
}

TEST(Iterator, RangeForModifiesElements) {
    pmr_list<int> l;
    for (int i = 1; i <= 5; ++i) l.emplace_back(i);
    for (auto& x : l) x *= 2;
    auto v = collectInts(l);
    EXPECT_EQ(v[0], 2);
    EXPECT_EQ(v.back(), 10);
}

TEST(Count, DistanceMatchesInsertions) {
    pmr_list<int> l;
    for (int i = 0; i < 17; ++i) l.emplace_back(i);
    size_t cnt = 0;
    for ([[maybe_unused]] auto const& x : l) ++cnt;
    EXPECT_EQ(cnt, 17u);
}

TEST(Stress, ManyAddRemove) {
    pmr_list<int> l;
    for (int i = 0; i < 500; ++i) l.emplace_back(i);
    for (int i = 0; i < 250; ++i) l.pop_back();
    for (int i = 0; i < 250; ++i) l.emplace_back(1000 + i);
    size_t cnt = 0;
    int sum = 0;
    for (auto const& x : l) { ++cnt; sum += x; }
    EXPECT_EQ(cnt, 500u);
    EXPECT_GT(sum, 0);
}

TEST(Reuse, ReuseMemoryAfterClear) {
    pmr_list<int> l;
    for (int i = 0; i < 200; ++i) l.emplace_back(i);
    for (int i = 0; i < 200; ++i) l.pop_back();
    for (int i = 0; i < 50; ++i) l.emplace_back(i + 1000);
    auto v = collectInts(l);
    ASSERT_EQ(v.size(), 50u);
    EXPECT_EQ(v.front(), 1000);
}

TEST(IteratorOps, PrePostIncrement) {
    pmr_list<int> l;
    l.emplace_back(1);
    l.emplace_back(2);
    auto it = l.begin();
    EXPECT_EQ(*it, 1);
    auto it2 = it++;
    EXPECT_EQ(*it2, 1);
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_TRUE(it == l.end());
}

TEST(ConstLikeIteration, IterationWithoutModification) {
    // реализация не предоставляет const begin/end — проверяем, что итерация без модификации возможна
    pmr_list<int> l;
    l.emplace_back(7);
    l.emplace_back(14);
    std::vector<int> out;
    for (auto const& x : l) out.push_back(x);
    EXPECT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], 14);
}

TEST(Clear, PopUntilEmpty) {
    pmr_list<int> l;
    l.emplace_back(1);
    l.emplace_back(2);
    l.emplace_back(3);
    while (true) {
        size_t c = 0;
        for ([[maybe_unused]] auto const& x : l) ++c;
        if (c == 0) break;
        l.pop_front();
    }
    size_t finalCount = 0;
    for ([[maybe_unused]] auto const& x : l) ++finalCount;
    EXPECT_EQ(finalCount, 0u);
}

TEST(Sum, InsertVariousIntsAndCheckSum) {
    pmr_list<int> l;
    std::vector<int> values = {3, 7, 11, 13, 17};
    for (int v : values) l.emplace_back(v);
    int sum = 0;
    for (int x : l) sum += x;
    EXPECT_EQ(sum, std::accumulate(values.begin(), values.end(), 0));
}

TEST(ModifyComplex, IncreaseSalary) {
    pmr_list<Employee> staff;
    staff.emplace_back(Employee("X", 20, 1000.0));
    staff.emplace_back(Employee("Y", 25, 2000.0));
    for (auto& e : staff) e.salary += 500.0;
    auto v = collectEmployees(staff);
    EXPECT_DOUBLE_EQ(v[0].salary, 1500.0);
    EXPECT_DOUBLE_EQ(v[1].salary, 2500.0);
}

TEST(IteratorCategory, IsForwardIteratorAtCompileTime) {
    pmr_list<int> l;
    using iter_t = decltype(l.begin());
    static_assert(std::is_same_v<typename std::iterator_traits<iter_t>::iterator_category,
                                 std::forward_iterator_tag>,
                  "Iterator must be forward iterator");
    SUCCEED();
}

TEST(Stability, MultipleClearsAreSafe) {
    pmr_list<int> l;
    for (int i = 0; i < 10; ++i) l.emplace_back(i);
    for (int r = 0; r < 5; ++r) {
        while (true) {
            size_t c = 0;
            for ([[maybe_unused]] auto const& x : l) ++c;
            if (c == 0) break;
            l.pop_back();
        }
    }
    size_t cnt = 0;
    for ([[maybe_unused]] auto const& x : l) ++cnt;
    EXPECT_EQ(cnt, 0u);
}

TEST(FrontBackAccess, AfterOperations) {
    pmr_list<int> l;
    l.emplace_back(10);
    l.emplace_back(20);
    l.emplace_front(5);
    EXPECT_EQ(collectInts(l).front(), 5);
    EXPECT_EQ(collectInts(l).back(), 20);
}

TEST(MultiType, UseWithDifferentTypes) {
    pmr_list<long> a;
    pmr_list<int> b;
    a.emplace_back(100L);
    b.emplace_back(7);
    EXPECT_EQ(collectInts(b)[0], 7);
    std::vector<long> va;
    for (auto x : a) va.push_back(x);
    EXPECT_EQ(va.size(), 1u);
    EXPECT_EQ(va[0], 100L);
}

TEST(RemoveEdge, RemoveLastElementThenAdd) {
    pmr_list<int> l;
    l.emplace_back(1);
    l.pop_back();
    EXPECT_EQ(collectInts(l).size(), 0u);
    l.emplace_back(42);
    EXPECT_EQ(collectInts(l).size(), 1u);
    EXPECT_EQ(collectInts(l)[0], 42);
}

TEST(InsertAfterRemove, RepeatedPushPop) {
    pmr_list<int> l;
    for (int i = 0; i < 20; ++i) l.emplace_back(i);
    for (int i = 0; i < 10; ++i) l.pop_front();
    for (int i = 0; i < 5; ++i) l.emplace_back(100 + i);
    size_t cnt = 0;
    for ([[maybe_unused]] auto const& x : l) ++cnt;
    EXPECT_EQ(cnt, 15u);
}

TEST(ComplexOrder, EmployeesOrderPreserved) {
    pmr_list<Employee> staff;
    staff.emplace_back(Employee("A", 20, 10.0));
    staff.emplace_front(Employee("B", 21, 11.0));
    staff.emplace_back(Employee("C", 22, 12.0));
    auto v = collectEmployees(staff);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0].name, "B");
    EXPECT_EQ(v[1].name, "A");
    EXPECT_EQ(v[2].name, "C");
}

TEST(Bulk, LargeNumberInsertions) {
    pmr_list<int> l;
    for (int i = 0; i < 1000; ++i) l.emplace_back(i);
    size_t cnt = 0;
    for ([[maybe_unused]] auto const& x : l) ++cnt;
    EXPECT_EQ(cnt, 1000u);
}

TEST(Consistency, ValuesRemainAfterMixedOps) {
    pmr_list<int> l;
    l.emplace_back(1); l.emplace_back(2); l.emplace_front(0);
    l.pop_back();
    l.emplace_back(9);
    auto v = collectInts(l);
    std::vector<int> expected = {0,1,9};
    EXPECT_EQ(v, expected);
}

TEST(IteratorValidity, DereferenceThenModify) {
    pmr_list<int> l;
    l.emplace_back(5);
    auto it = l.begin();
    *it = 42;
    EXPECT_EQ(collectInts(l)[0], 42);
}

TEST(Edge, EmplaceFrontOnEmptyWorks) {
    pmr_list<int> l;
    l.emplace_front(111);
    EXPECT_EQ(collectInts(l).size(), 1u);
    EXPECT_EQ(collectInts(l)[0], 111);
}

TEST(Edge, RemoveWithManySmallOperations) {
    pmr_list<int> l;
    for (int i = 0; i < 50; ++i) l.emplace_back(i);
    for (int i = 0; i < 25; ++i) { l.pop_front(); l.emplace_back(1000 + i); }
    size_t cnt = 0;
    for ([[maybe_unused]] auto const& x : l) ++cnt;
    EXPECT_EQ(cnt, 50u);
}
