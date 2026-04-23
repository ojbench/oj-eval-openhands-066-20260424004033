
#include "src.hpp"
#include <iostream>
#include <map>
#include <vector>

int main() {
    // Test example1
    {
        sjtu::any_ptr a = sjtu::make_any_ptr(int(1));
        sjtu::any_ptr b = a;

        a.unwrap<int>() = 2;
        std::cout << b.unwrap<int>() << std::endl;  // 2

        b = new std::string;
        b.unwrap<std::string>() = "Hello, world!";
        std::cout << b.unwrap<std::string>() << std::endl;  // Hello, world!

        try {
            a.unwrap<std::string>() = "a";
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;  // bad cast
        }
    }

    // Test example2
    {
        sjtu::any_ptr a = sjtu::make_any_ptr(1);
        sjtu::any_ptr v = sjtu::make_any_ptr<std::vector<int>>({1, 2, 3});
        sjtu::any_ptr m = sjtu::make_any_ptr<std::map<int, int>>({{1, 2}, {3, 4}});
        std::cout << a.unwrap<int>() << std::endl;  // 1
        std::cout << v.unwrap<std::vector<int>>().size() << std::endl;  // 3
        std::cout << m.unwrap<std::map<int, int>>().size() << std::endl;  // 2
    }

    return 0;
}
