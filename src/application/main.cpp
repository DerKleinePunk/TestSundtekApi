#include <iostream>
#include "../libary/sundtekApi.hpp"

int main(int, char**) {
    std::cout << "Hello, world! I Try the sundTek Api with you" << std::endl;

    const auto master = new sundtekApi();
    auto result = master->searchDevices();
    if(result > 0) {
        std::cout << "one or more devices found" << std::endl;
        result = master->searchFmStations();
        if(result == 0) {
            std::cout << "fm search ready" << std::endl;
        }
    }
    else if(result== 0) {
        std::cout << "no error but no devices found" << std::endl;
    }
    delete master;
}
