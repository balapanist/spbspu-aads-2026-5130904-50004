#ifndef KHVAEVSKIY_COMMANDS_HPP
#define KHVAEVSKIY_COMMANDS_HPP

#include <iosfwd>
#include <hash_table.hpp>
#include "command_parser.hpp"
#include "network.hpp"

namespace khvaevskiy
{
  class Commands
  {
  public:
    Commands();
    bool execute(Network & network, const Arguments & arguments, std::ostream & output) const;

  private:
    using Argument = Arguments::const_iterator;
    using Handler = void (*)(Network &, Argument, std::ostream &);

    struct Command
    {
      std::size_t arity;
      Handler handler;
    };

    HashTable< std::string, Command > commands_;

    static void add_device(Network & network, Argument argument, std::ostream & output);
    static void remove_device(Network & network, Argument argument, std::ostream & output);
    static void show_device(Network & network, Argument argument, std::ostream & output);
    static void connect(Network & network, Argument argument, std::ostream & output);
    static void disconnect(Network & network, Argument argument, std::ostream & output);
    static void rename(Network & network, Argument argument, std::ostream & output);
    static void set_ip(Network & network, Argument argument, std::ostream & output);
    static void neighbors(Network & network, Argument argument, std::ostream & output);
    static void route(Network & network, Argument argument, std::ostream & output);
    static void show_network(Network & network, Argument argument, std::ostream & output);
    static void stats(Network & network, Argument argument, std::ostream & output);
    static void clear(Network & network, Argument argument, std::ostream & output);
    static void help(Network & network, Argument argument, std::ostream & output);
  };
} // namespace khvaevskiy

#endif
