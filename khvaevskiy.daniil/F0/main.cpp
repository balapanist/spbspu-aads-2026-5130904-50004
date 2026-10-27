#include <exception>
#include <iostream>
#include "console.hpp"

int main()
{
  try
  {
    khvaevskiy::Network network;
    khvaevskiy::run_console(network, std::cin, std::cout);
  }
  catch (const std::exception & error)
  {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return 1;
  }
}
