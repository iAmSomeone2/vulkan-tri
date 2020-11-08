#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#include "TriangleApp.hpp"

#include <iostream>
#include <stdexcept>

#include <fmt/format.h>

using namespace VkTri;

void errHandler(int num, const char* desc)
{
    throw std::runtime_error(fmt::format(FMT_STRING("GLFW Error ({:d}): {:s}"), num, desc));
}

int main()
{
    // Init the library
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW.\n";
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(errHandler);

    auto triApp = TriangleApp::create();

    triApp->run();

    glfwTerminate();
    return EXIT_SUCCESS;
}
