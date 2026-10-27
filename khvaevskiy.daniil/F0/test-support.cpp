#include "test-support.hpp"

khvaevskiy::Network khvaevskiy::make_network(int count)
{
  Network network;
  for (int i = 0; i < count; ++i)
  {
    network.add_device("n" + std::to_string(i), "10.0.0." + std::to_string(i));
  }
  return network;
}

void khvaevskiy::verify_paths(const Network & network, const RouteResult & result, const std::string & source,
                              const std::string & sink)
{
  BOOST_REQUIRE(result.possible);
  std::map< std::pair< std::string, std::string >, Capacity > capacities;
  std::map< std::pair< std::string, std::string >, Capacity > used;
  for (auto edge = network.edges_begin(); edge != network.edges_end(); ++edge)
  {
    capacities[{edge->first, edge->second}] = edge->capacity;
  }
  Capacity total = 0;
  for (const auto & path: result.paths)
  {
    BOOST_REQUIRE(path.amount > 0);
    BOOST_REQUIRE(path.devices.size() >= 2);
    auto vertex = path.devices.begin();
    BOOST_CHECK(*vertex == source);
    std::string previous = *vertex++;
    std::set< std::string > visited;
    visited.insert(previous);
    for (; vertex != path.devices.end(); ++vertex)
    {
      BOOST_CHECK(visited.insert(*vertex).second);
      auto first = previous;
      auto second = *vertex;
      if (second < first)
      {
        first.swap(second);
      }
      const auto key = std::make_pair(first, second);
      BOOST_REQUIRE(capacities.count(key) == 1);
      used[key] += path.amount;
      BOOST_CHECK(used[key] <= capacities.at(key));
      previous = *vertex;
    }
    BOOST_CHECK(previous == sink);
    total += path.amount;
  }
  BOOST_CHECK(total == result.requested);
  BOOST_CHECK(total == result.transferred);
}
