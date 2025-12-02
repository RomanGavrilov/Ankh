#include <iostream>

#include "app/application.hpp"
#include "utils/logging.hpp"

int main()
{
    try
    {
        ankh::log::init();
        ankh::Application app;
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
