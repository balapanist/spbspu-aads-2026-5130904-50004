#ifndef KHVAEVSKIY_OUTPUT_HPP
#define KHVAEVSKIY_OUTPUT_HPP

#include <iosfwd>
#include "network.hpp"

namespace khvaevskiy
{
  void print_device(const DeviceInfo & device, std::ostream & output);
  void print_network(const Network & network, std::ostream & output);
  void print_route(const RouteResult & result, std::ostream & output);
  void print_help(std::ostream & output);
} // namespace khvaevskiy

#endif
