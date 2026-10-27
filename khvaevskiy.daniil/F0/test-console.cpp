#include "test-support.hpp"

namespace khvaevskiy
{
  void console_integration()
  {
    khvaevskiy::Network network;
    std::istringstream input("\nadd-device A 10.0.0.1\nadd-device B 10.0.0.2\nconnect A B 2\nconnect B A 3\n"
                             "route A B 5\nroute A B 6\nconnect A B 1oops\nroute A B 99999999999999999999999\n"
                             "connect A B 2 extra\nrename\nunknown\nshow-device A\nneighbors B\n"
                             "rename B C\nset-ip C 10.0.0.3\nshow-network\nstats\nhelp\ndisconnect A C\n"
                             "remove-device C\nclear\nexit extra\nexit\nadd-device ignored 10.0.0.4\n");
    std::ostringstream output;
    khvaevskiy::run_console(network, input, output, false);
    const auto text = output.str();
    BOOST_REQUIRE_MESSAGE(text.find("A -> B : 5") != std::string::npos, "Console path");
    BOOST_REQUIRE_MESSAGE(text.find("IMPOSSIBLE: requested 6, maximum 5") != std::string::npos,
                          "Console infeasibility");
    BOOST_REQUIRE_MESSAGE(text.find("CONNECTIONS: 1") != std::string::npos, "No duplicate edges through console");
    std::size_t errors = 0, at = 0;
    while ((at = text.find("ERROR:", at)) != std::string::npos)
    {
      ++errors;
      at += 6;
    }
    BOOST_REQUIRE_MESSAGE(errors == 6, "Console strict parsing and recovery");
    BOOST_REQUIRE_MESSAGE(network.device_count() == 0, "Exit stops processing");
    std::istringstream eof("add-device A 10.0.0.1");
    khvaevskiy::run_console(network, eof, output, false);
    BOOST_REQUIRE_MESSAGE(network.device_count() == 1, "EOF without newline");
  }
} // namespace khvaevskiy

BOOST_AUTO_TEST_CASE(console_integration)
{
  khvaevskiy::console_integration();
}
