#include "stdafx.h"

#include <iostream>

int main(const int argc, char* argv[])
{
    for (auto i = 1; i < argc; ++i)
    {
        const auto arg = argv[i];

        if (arg[0] != '-' || arg[1] != '-')
        {
            std::cout << "Invalid argument \"" << arg << "\"" << std::endl;
            return -1;
        }

        const auto str = &arg[2];

        if (strcmp(str, "lazy") == 0)
        {
            
        }
    }

    std::cin.get();

    return 0;
}
