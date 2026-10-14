#include <iostream>
#include <string_view>
#include "uavsim/version.hpp"

int main() {
    constexpr std::string_view standard = "C++20";
    std::cout << "UAVSim " << UAVSIM_VERSION_STRING << " (" << standard << ")\n";
    return 0;
}
