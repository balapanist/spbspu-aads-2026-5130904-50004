#include "commands.hpp"
#include <ostream>
#include <stdexcept>
#include "output.hpp"

khvaevskiy::Commands::Commands()
{
  commands_.insert("add-device", Command{2, add_device});
  commands_.insert("remove-device", Command{1, remove_device});
  commands_.insert("show-device", Command{1, show_device});
  commands_.insert("connect", Command{3, connect});
  commands_.insert("disconnect", Command{2, disconnect});
  commands_.insert("rename", Command{2, rename});
  commands_.insert("set-ip", Command{2, set_ip});
  commands_.insert("neighbors", Command{1, neighbors});
  commands_.insert("route", Command{3, route});
  commands_.insert("show-network", Command{0, show_network});
  commands_.insert("stats", Command{0, stats});
  commands_.insert("clear", Command{0, clear});
  commands_.insert("help", Command{0, help});
}

bool khvaevskiy::Commands::execute(Network & network, const Arguments & arguments, std::ostream & output) const
{
  if (arguments.empty())
  {
    return true;
  }
  auto argument = arguments.begin();
  if (*argument == "exit")
  {
    if (arguments.size() != 1)
    {
      throw std::invalid_argument("Wrong argument count; type help");
    }
    return false;
  }
  const auto command = commands_.find(*argument);
  if (command == commands_.end())
  {
    throw std::invalid_argument("Unknown command; type help");
  }
  if (arguments.size() != command->second.arity + 1)
  {
    throw std::invalid_argument("Wrong argument count; type help");
  }
  command->second.handler(network, ++argument, output);
  return true;
}

void khvaevskiy::Commands::add_device(Network & network, Argument argument, std::ostream & output)
{
  const auto & name = *argument++;
  network.add_device(name, *argument);
  output << "OK\n";
}

void khvaevskiy::Commands::remove_device(Network & network, Argument argument, std::ostream & output)
{
  network.remove_device(*argument);
  output << "OK\n";
}

void khvaevskiy::Commands::show_device(Network & network, Argument argument, std::ostream & output)
{
  print_device(network.device(*argument), output);
}

void khvaevskiy::Commands::connect(Network & network, Argument argument, std::ostream & output)
{
  const auto & first = *argument++;
  const auto & second = *argument++;
  network.connect(first, second, parse_capacity(*argument));
  output << "OK\n";
}

void khvaevskiy::Commands::disconnect(Network & network, Argument argument, std::ostream & output)
{
  const auto & first = *argument++;
  network.disconnect(first, *argument);
  output << "OK\n";
}

void khvaevskiy::Commands::rename(Network & network, Argument argument, std::ostream & output)
{
  const auto & name = *argument++;
  network.rename_device(name, *argument);
  output << "OK\n";
}

void khvaevskiy::Commands::set_ip(Network & network, Argument argument, std::ostream & output)
{
  const auto & name = *argument++;
  network.set_ip(name, *argument);
  output << "OK\n";
}

void khvaevskiy::Commands::neighbors(Network & network, Argument argument, std::ostream & output)
{
  const auto result = network.neighbors(*argument);
  for (const auto & neighbor: result)
  {
    output << neighbor.name << " : " << neighbor.capacity << '\n';
  }
}

void khvaevskiy::Commands::route(Network & network, Argument argument, std::ostream & output)
{
  const auto & source = *argument++;
  const auto & sink = *argument++;
  const auto result = network.route(source, sink, parse_capacity(*argument));
  print_route(result, output);
}

void khvaevskiy::Commands::show_network(Network & network, Argument, std::ostream & output)
{
  print_network(network, output);
}

void khvaevskiy::Commands::stats(Network & network, Argument, std::ostream & output)
{
  output << "DEVICES: " << network.device_count() << ", CONNECTIONS: " << network.edge_count() << '\n';
}

void khvaevskiy::Commands::clear(Network & network, Argument, std::ostream & output)
{
  network.clear();
  output << "OK\n";
}

void khvaevskiy::Commands::help(Network &, Argument, std::ostream & output)
{
  print_help(output);
}
