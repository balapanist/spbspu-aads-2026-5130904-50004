#include "test-support.hpp"

namespace khvaevskiy
{
  void device_validation()
  {
    khvaevskiy::Network network;
    network.add_device("pc1", "192.168.0.1");
    throws([&] { network.add_device("pc1", "192.168.0.2"); });
    throws([&] { network.add_device("pc2", "192.168.0.1"); });
    for (const auto & bad: {"", "256.0.0.1", "1.2.3", "1.2.3.4.5", "1.2.3.", "01.2.3.4", "1.-2.3.4", "a.2.3.4"})
    {
      throws([&] { network.add_device("bad", bad); });
    }
    for (const auto & bad: {"", "a b", "a.b", "a/b"})
    {
      throws([&] { network.add_device(bad, "10.0.0.2"); });
    }
    throws([&] { network.add_device(std::string(65, 'a'), "10.0.0.2"); });
    network.add_device("pc2", "192.168.0.2");
    throws([&] { network.set_ip("pc1", "192.168.0.2"); });
    network.set_ip("pc1", "192.168.0.1");
    network.set_ip("pc1", "10.1.2.3");
    BOOST_REQUIRE_MESSAGE(network.device("pc1").ip == "10.1.2.3", "set-ip");
    throws([&] { (void)network.device("missing"); });
    BOOST_REQUIRE_MESSAGE(network.device_count() == 2, "Invalid commands preserve graph");
  }

  void edge_lifecycle()
  {
    auto network = make_network(3);
    network.connect("n0", "n1", 2);
    network.connect("n1", "n0", 3);
    BOOST_REQUIRE_MESSAGE(network.edge_count() == 1 && network.neighbors("n0").front().capacity == 5,
                          "Repeat connect increments one edge");
    BOOST_REQUIRE_MESSAGE(network.device("n0").connections == 1 && network.device("n1").connections == 1,
                          "Undirected degree");
    verify_paths(network, network.route("n1", "n0", 5), "n1", "n0");
    throws([&] { network.connect("n0", "n0", 2); });
    throws([&] { network.connect("n0", "missing", 2); });
    throws([&] { network.connect("n0", "n1", 0); });
    throws([&] { network.connect("n0", "n1", -1); });
    network.disconnect("n1", "n0");
    BOOST_REQUIRE_MESSAGE(network.edge_count() == 0, "Disconnect removes whole edge");
    throws([&] { network.disconnect("n0", "n1"); });
    network.connect("n0", "n1", 1);
    network.connect("n1", "n2", 2);
    network.remove_device("n1");
    BOOST_REQUIRE_MESSAGE(network.device_count() == 2 && network.edge_count() == 0, "Remove deletes incident edges");
    BOOST_REQUIRE_MESSAGE(network.device("n0").connections == 0, "No dangling neighbor");
    network.add_device("n1", "10.0.0.1");
    network.clear();
    BOOST_REQUIRE_MESSAGE(network.device_count() == 0 && network.edge_count() == 0, "Clear network");
  }

  void rename_preserves_graph()
  {
    auto network = make_network(3);
    network.connect("n0", "n1", 4);
    network.connect("n1", "n2", 4);
    throws([&] { network.rename_device("n1", "n0"); });
    network.rename_device("n1", "z");
    BOOST_REQUIRE_MESSAGE(network.device("z").ip == "10.0.0.1" && network.device("z").connections == 2,
                          "Rename properties");
    throws([&] { (void)network.device("n1"); });
    verify_paths(network, network.route("n0", "n2", 4), "n0", "n2");
    network.connect("n0", "z", 1);
    BOOST_REQUIRE_MESSAGE(network.edge_count() == 2, "Rename retains edge identity");
    network.rename_device("z", "z");
    network.disconnect("z", "n0");
    BOOST_REQUIRE_MESSAGE(network.device("z").connections == 1, "Disconnect after rename");
  }
} // namespace khvaevskiy

BOOST_AUTO_TEST_CASE(device_validation)
{
  khvaevskiy::device_validation();
}

BOOST_AUTO_TEST_CASE(edge_lifecycle)
{
  khvaevskiy::edge_lifecycle();
}

BOOST_AUTO_TEST_CASE(rename_preserves_graph)
{
  khvaevskiy::rename_preserves_graph();
}
