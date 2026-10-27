#include "test-support.hpp"

namespace khvaevskiy
{
  void split_flow()
  {
    auto network = make_network(4);
    network.connect("n0", "n1", 2);
    network.connect("n1", "n3", 2);
    network.connect("n0", "n2", 3);
    network.connect("n2", "n3", 3);
    const auto result = network.route("n0", "n3", 5);
    verify_paths(network, result, "n0", "n3");
    BOOST_REQUIRE_MESSAGE(result.paths.size() == 2 &&
                            (result.paths.front().amount == 2 || result.paths.front().amount == 3),
                          "Split 2 + 3");
    const auto failed = network.route("n0", "n3", 6);
    BOOST_REQUIRE_MESSAGE(!failed.possible && failed.transferred == 5 && failed.paths.empty(), "Impossible maximum");
    verify_paths(network, network.route("n0", "n3", 4), "n0", "n3");
    verify_paths(network, network.route("n0", "n3", 5), "n0", "n3");
    verify_paths(network, network.route("n3", "n0", 5), "n3", "n0");
  }

  void residual_rerouting()
  {
    // Select labels that force the first BFS through the crossing edge.
    // Hash iteration order differs between standard library implementations.
    std::vector< std::string > labels{"n1", "n2", "n3", "n4"};
    bool forced = false;
    do
    {
      auto network = make_network(6);
      const auto & a = labels[0];
      const auto & b = labels[1];
      const auto & c = labels[2];
      const auto & d = labels[3];
      network.connect("n0", a, 1);
      network.connect("n0", b, 1);
      network.connect(a, c, 1);
      network.connect(a, d, 1);
      network.connect(b, c, 1);
      network.connect(c, "n5", 1);
      network.connect(d, "n5", 1);
      const auto first = network.route("n0", "n5", 1);
      BOOST_REQUIRE(first.paths.size() == 1);
      const auto & path = first.paths.front().devices;
      const std::vector< std::string > observed(path.begin(), path.end());
      const std::vector< std::string > crossing{"n0", a, c, "n5"};
      if (observed == crossing)
      {
        // A second unit now requires cancelling a->c via the reverse residual edge.
        const auto result = network.route("n0", "n5", 2);
        verify_paths(network, result, "n0", "n5");
        BOOST_REQUIRE(result.paths.size() == 2);
        forced = true;
        break;
      }
    } while (std::next_permutation(labels.begin(), labels.end()));
    BOOST_REQUIRE_MESSAGE(forced, "The test must exercise residual cancellation");
  }

  void boundaries()
  {
    auto network = make_network(3);
    BOOST_REQUIRE_MESSAGE(!network.route("n0", "n1", 1).possible && network.route("n0", "n1", 1).transferred == 0,
                          "Disconnected");
    throws([&] { (void)network.route("n0", "n0", 1); });
    throws([&] { (void)network.route("n0", "missing", 1); });
    throws([&] { (void)network.route("n0", "n1", 0); });
    throws([&] { (void)network.route("n0", "n1", -1); });
    throws([&] { (void)network.route("n0", "n1", khvaevskiy::max_capacity + 1); });
    network.connect("n0", "n1", khvaevskiy::max_capacity);
    throws< std::overflow_error >([&] { network.connect("n0", "n1", 1); });
    BOOST_REQUIRE_MESSAGE(network.neighbors("n0").front().capacity == khvaevskiy::max_capacity,
                          "Overflow leaves edge unchanged");
    verify_paths(network, network.route("n0", "n1", khvaevskiy::max_capacity), "n0", "n1");
    network.connect("n1", "n2", khvaevskiy::max_capacity);
    verify_paths(network, network.route("n2", "n0", khvaevskiy::max_capacity), "n2", "n0");
  }

  void random_mincut_oracle()
  {
    std::mt19937 random(20261027);
    constexpr int n = 6;
    for (int trial = 0; trial < 200; ++trial)
    {
      auto network = make_network(n);
      khvaevskiy::Capacity capacities[n][n]{};
      for (int i = 0; i < n; ++i)
      {
        for (int j = i + 1; j < n; ++j)
        {
          auto c = static_cast< khvaevskiy::Capacity >(random() % 5);
          if (c == 0)
          {
            continue;
          }
          capacities[i][j] = capacities[j][i] = c;
          network.connect("n" + std::to_string(i), "n" + std::to_string(j), c);
        }
      }
      // Independent oracle: enumerate all cuts containing source but not sink.
      auto cut = khvaevskiy::max_capacity;
      for (unsigned mask = 1; mask < (1u << n); ++mask)
      {
        if (!(mask & 1u) || (mask & (1u << (n - 1))))
        {
          continue;
        }
        khvaevskiy::Capacity sum = 0;
        for (int i = 0; i < n; ++i)
        {
          for (int j = i + 1; j < n; ++j)
          {
            if (((mask >> i) & 1u) != ((mask >> j) & 1u))
            {
              sum += capacities[i][j];
            }
          }
        }
        cut = std::min(cut, sum);
      }
      const auto impossible = network.route("n0", "n5", cut + 1);
      BOOST_REQUIRE_MESSAGE(!impossible.possible && impossible.transferred == cut && impossible.paths.empty(),
                            "Max-flow equals enumerated min-cut");
      if (cut > 0)
      {
        verify_paths(network, network.route("n0", "n5", cut), "n0", "n5");
        verify_paths(network, network.route("n5", "n0", cut), "n5", "n0");
        verify_paths(network, network.route("n0", "n5", std::max< khvaevskiy::Capacity >(1, cut / 2)), "n0", "n5");
      }
    }
  }
} // namespace khvaevskiy

BOOST_AUTO_TEST_CASE(split_flow)
{
  khvaevskiy::split_flow();
}

BOOST_AUTO_TEST_CASE(residual_rerouting)
{
  khvaevskiy::residual_rerouting();
}

BOOST_AUTO_TEST_CASE(boundaries)
{
  khvaevskiy::boundaries();
}

BOOST_AUTO_TEST_CASE(random_mincut_oracle)
{
  khvaevskiy::random_mincut_oracle();
}
