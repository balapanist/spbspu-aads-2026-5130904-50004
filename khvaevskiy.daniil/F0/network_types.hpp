#ifndef KHVAEVSKIY_NETWORK_TYPES_HPP
#define KHVAEVSKIY_NETWORK_TYPES_HPP

#include <cstdint>
#include <limits>
#include <string>
#include <list.hpp>

namespace khvaevskiy
{
  using Capacity = std::int64_t;
  constexpr Capacity max_capacity = std::numeric_limits< Capacity >::max() / 2;

  struct Device
  {
    std::string ip;
  };

  struct Edge
  {
    std::string first;
    std::string second;
    Capacity capacity;
  };

  struct DeviceInfo
  {
    std::string name;
    std::string ip;
    std::size_t connections;
  };

  struct Neighbor
  {
    std::string name;
    Capacity capacity;
  };

  struct FlowPath
  {
    List< std::string > devices;
    Capacity amount;
  };

  struct RouteResult
  {
    bool possible;
    Capacity requested;
    Capacity transferred;
    List< FlowPath > paths;
  };
} // namespace khvaevskiy

#endif
