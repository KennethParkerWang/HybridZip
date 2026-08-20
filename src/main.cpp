#include <exception>
#include <iostream>

#include "app/cli.h"

int main(int argc, char* argv[]) {
    try {
        return hz::run_cli(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "hybridzip: " << error.what() << '\n';
        return 1;
    }
}
