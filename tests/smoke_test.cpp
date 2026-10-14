#include <iostream>
#include <string_view>
#include "uavsim/version.hpp"

int main() {
    constexpr std::string_view version = UAVSIM_VERSION_STRING;
    if (version.empty() || version == "0.0.0") {
        std::cerr << "invalid version\n";
        return 1;
    }
    std::cout << "smoke: version=" << version << "\n";
    return 0;
}
