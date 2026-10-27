#include "validation.hpp"
#include <stdexcept>

namespace khvaevskiy
{
  void validate_name(const std::string & name)
  {
    if (name.empty() || name.size() > 64)
    {
      throw std::invalid_argument("Name must contain 1..64 ASCII letters, digits, '_' or '-'");
    }
    for (unsigned char c: name)
    {
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-'))
      {
        throw std::invalid_argument("Invalid device name");
      }
    }
  }
  void validate_ip(const std::string & ip)
  {
    std::size_t start = 0;
    for (int part = 0; part < 4; ++part)
    {
      const auto dot = ip.find('.', start);
      const auto end = dot == std::string::npos ? ip.size() : dot;
      const auto length = end - start;
      if (length == 0 || length > 3 || (length > 1 && ip[start] == '0') || ((part == 3) != (dot == std::string::npos)))
      {
        throw std::invalid_argument("Expected IPv4, e.g. 192.168.0.1 (no leading zeros)");
      }
      unsigned value = 0;
      for (auto i = start; i < end; ++i)
      {
        if (ip[i] < '0' || ip[i] > '9')
        {
          throw std::invalid_argument("Invalid IPv4");
        }
        value = value * 10 + static_cast< unsigned >(ip[i] - '0');
      }
      if (value > 255)
      {
        throw std::invalid_argument("IPv4 octet exceeds 255");
      }
      start = end + 1;
    }
  }
  void validate_capacity(Capacity n)
  {
    if (n <= 0 || n > max_capacity)
    {
      throw std::invalid_argument("Capacity/flow must be in 1..4611686018427387903");
    }
  }
  bool same_edge(const Edge & edge, const std::string & a, const std::string & b)
  {
    return (edge.first == a && edge.second == b) || (edge.first == b && edge.second == a);
  }
} // namespace khvaevskiy
