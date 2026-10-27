#include "command_parser.hpp"
#include <stdexcept>

khvaevskiy::Arguments khvaevskiy::parse_command(const std::string & line)
{
  Arguments result;
  std::size_t start = 0;
  while (start < line.size())
  {
    start = line.find_first_not_of(" \t\r\n\v\f", start);
    if (start == std::string::npos)
    {
      break;
    }
    const auto end = line.find_first_of(" \t\r\n\v\f", start);
    if (end == std::string::npos)
    {
      result.push_back(line.substr(start));
      break;
    }
    result.push_back(line.substr(start, end - start));
    start = end;
  }
  return result;
}

khvaevskiy::Capacity khvaevskiy::parse_capacity(const std::string & token)
{
  if (token.empty())
  {
    throw std::invalid_argument("Expected a positive integer");
  }
  Capacity value = 0;
  for (char digit: token)
  {
    if (digit < '0' || digit > '9')
    {
      throw std::invalid_argument("Expected a positive integer");
    }
    const Capacity part = digit - '0';
    if (value > (max_capacity - part) / 10)
    {
      throw std::out_of_range("Capacity exceeds supported range");
    }
    value = value * 10 + part;
  }
  if (value == 0)
  {
    throw std::invalid_argument("Capacity must be positive");
  }
  return value;
}
