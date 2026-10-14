#include <algorithm>
#include <concepts>
#include <iostream>
#include <ranges>
#include <span>
#include <string_view>
#include "uavsim/version.hpp"

template<std::integral T> constexpr T twice(T value) { return value + value; }
static_assert(__cplusplus >= 202002L);
static_assert(twice(21) == 42);

int main(int argc, char** argv) {
    if (argc > 2 || (argc == 2 && std::string_view(argv[1]) != "--self-test")) {
        std::cerr << "ERROR: expected no argument or --self-test\n";
        return 2;
    }
    int values[]{1, 2, 3, 4};
    std::span<int> view(values);
    auto even = view | std::views::filter([](int n) { return n % 2 == 0; });
    int sum = 0;
    for (int n : even) sum += n;
    if (sum != 6 || view.size() != 4 || twice(3) != 6) {
        std::cerr << "ERROR: C++20 span/ranges/concepts smoke test failed\n";
        return 1;
    }
    std::cout << "UAVSim " << UAVSIM_VERSION_STRING << " C++=" << __cplusplus
              << " pointer_bits=" << sizeof(void*) * 8
#ifdef _MSC_VER
              << " MSVC=" << _MSC_FULL_VER
#elif defined(__clang__)
              << " Clang=" << __clang_version__
#endif
              << " self_test=PASS\n";
    return 0;
}
