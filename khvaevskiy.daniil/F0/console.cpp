#include "console.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include "command_parser.hpp"
#include "commands.hpp"

void khvaevskiy::run_console(Network & network, std::istream & input, std::ostream & output, bool interactive)
{
  Commands commands;
  if (interactive)
  {
    output << "Network Model. Type help for commands.\n";
  }
  std::string line;
  while (true)
  {
    if (interactive)
    {
      output << "> " << std::flush;
    }
    if (!std::getline(input, line))
    {
      if (input.bad() || !input.eof())
      {
        throw std::runtime_error("Input failure");
      }
      break;
    }
    try
    {
      if (!commands.execute(network, parse_command(line), output))
      {
        break;
      }
    }
    catch (const std::invalid_argument & error)
    {
      output << "ERROR: " << error.what() << '\n';
    }
    catch (const std::out_of_range & error)
    {
      output << "ERROR: " << error.what() << '\n';
    }
    catch (const std::overflow_error & error)
    {
      output << "ERROR: " << error.what() << '\n';
    }
    if (!output)
    {
      throw std::runtime_error("Output failure");
    }
  }
}
