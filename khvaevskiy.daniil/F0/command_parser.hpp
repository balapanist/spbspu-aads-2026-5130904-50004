#ifndef KHVAEVSKIY_COMMAND_PARSER_HPP
#define KHVAEVSKIY_COMMAND_PARSER_HPP

#include <string>
#include <list.hpp>
#include "network_types.hpp"

namespace khvaevskiy
{
  using Arguments = List< std::string >;
  Arguments parse_command(const std::string & line);
  Capacity parse_capacity(const std::string & token);
} // namespace khvaevskiy

#endif
