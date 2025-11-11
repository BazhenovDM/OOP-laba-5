#include <iostream>
#include <string>
#include <memory_resource>
#include "include/pool_resource.hpp"
#include "include/pmr_list.hpp"

struct Employee {
    std::string name;
    int age;
    double salary;
    Employee() = default;
    Employee(std::string n, int a, double s): name(std::move(n)), age(a), salary(s) {}
};

int main() {
    std::size_t buf = 64 * 1024;
    FixedListMemoryResource mr(buf);
    using IntList = pmr_list<int>;
    IntList ints(&mr);
    for (int i = 1; i <= 10; ++i) ints.push_back(i * 10);
    std::cout << "ints:";
    for (auto it = ints.begin(); it != ints.end(); ++it) std::cout << ' ' << *it;
    std::cout << '\n';
    using EmpList = pmr_list<Employee>;
    EmpList staff(&mr);
    staff.emplace_back(Employee("Alice", 30, 60000.0));
    staff.emplace_back(Employee("Bob", 28, 52000.0));
    staff.emplace_front(Employee("Zara", 35, 80000.0));
    std::cout << "staff:\n";
    for (auto it = staff.begin(); it != staff.end(); ++it) {
        std::cout << it->name << " age=" << it->age << " salary=" << it->salary << '\n';
    }
    for (int i = 0; i < 3; ++i) ints.pop_back();
    std::cout << "ints after pop:";
    for (auto it = ints.begin(); it != ints.end(); ++it) std::cout << ' ' << *it;
    std::cout << '\n';
    staff.pop_front();
    std::cout << "staff after pop_front:\n";
    for (auto it = staff.begin(); it != staff.end(); ++it) {
        std::cout << it->name << " age=" << it->age << '\n';
    }
    return 0;
}
