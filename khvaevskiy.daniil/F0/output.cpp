#include "output.hpp"
#include <ostream>

void khvaevskiy::print_device(const DeviceInfo & device, std::ostream & output)
{
  output << "<NAME: " << device.name << ", IP: " << device.ip;
  output << ", CONNECTIONS: " << device.connections << ">\n";
}

void khvaevskiy::print_network(const Network & network, std::ostream & output)
{
  for (const auto & entry: network)
  {
    print_device(network.device(entry.first), output);
  }
  for (auto edge = network.edges_begin(); edge != network.edges_end(); ++edge)
  {
    output << edge->first << " -- " << edge->second << " : " << edge->capacity << '\n';
  }
}

void khvaevskiy::print_route(const RouteResult & result, std::ostream & output)
{
  if (!result.possible)
  {
    output << "IMPOSSIBLE: requested " << result.requested << ", maximum " << result.transferred << '\n';
    return;
  }
  output << "FLOW: " << result.transferred << '\n';
  for (const auto & path: result.paths)
  {
    auto vertex = path.devices.begin();
    output << *vertex;
    while (++vertex != path.devices.end())
    {
      output << " -> " << *vertex;
    }
    output << " : " << path.amount << '\n';
  }
}

void khvaevskiy::print_help(std::ostream & output)
{
  output << "add-device <name> <ip>\nremove-device <name>\nshow-device <name>\n";
  output << "connect <device1> <device2> <capacity>\ndisconnect <device1> <device2>\n";
  output << "neighbors <name>\nroute <device1> <device2> <N>\n";
  output << "rename <old-name> <new-name>\nset-ip <name> <ip>\n";
  output << "show-network\nstats\nclear\nhelp\nexit\n";
}
