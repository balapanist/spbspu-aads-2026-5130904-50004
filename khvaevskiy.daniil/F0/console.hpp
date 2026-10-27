#ifndef KHVAEVSKIY_CONSOLE_HPP
#define KHVAEVSKIY_CONSOLE_HPP

#include <iosfwd>
#include "network.hpp"

namespace khvaevskiy
{
  void run_console(Network & network, std::istream & input, std::ostream & output, bool interactive = true);
}

#endif
