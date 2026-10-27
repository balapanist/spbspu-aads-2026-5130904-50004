#ifndef KHVAEVSKIY_VALIDATION_HPP
#define KHVAEVSKIY_VALIDATION_HPP

#include <string>
#include "network_types.hpp"

namespace khvaevskiy
{
  void validate_name(const std::string & name);
  void validate_ip(const std::string & ip);
  void validate_capacity(Capacity capacity);
  bool same_edge(const Edge & edge, const std::string & first, const std::string & second);
} // namespace khvaevskiy

#endif
