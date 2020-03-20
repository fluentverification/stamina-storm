#include "StaminaCL.h"





int main(int argc, char* argv[]) {

    if (argc > 1) {
        StaminaCL::parseArguments(argc, argv);
        std::cout << "Called with parameters" << std::endl;
    }
    else {
        std::cout << "Error: no parameters" << std::endl;
    }
    return 0;
}



