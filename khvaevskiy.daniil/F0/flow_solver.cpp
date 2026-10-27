#include "flow_solver.hpp"
#include <stdexcept>
#include "validation.hpp"

khvaevskiy::RouteResult khvaevskiy::Network::route(const std::string & source, const std::string & sink,
                                                   Capacity required) const
{
  require_device(source);
  require_device(sink);
  validate_capacity(required);
  if (source == sink)
  {
    throw std::invalid_argument("Route endpoints must be different");
  }
  FlowSolver solver(*this);
  return solver.solve(source, sink, required);
}

khvaevskiy::FlowSolver::FlowSolver(const Network & network)
{
  for (const auto & device: network)
  {
    graph_.insert(device.first, HashTable< std::string, Channel >());
  }
  for (auto edge = network.edges_begin(); edge != network.edges_end(); ++edge)
  {
    graph_.at(edge->first).insert(edge->second, Channel{edge->capacity, 0});
    graph_.at(edge->second).insert(edge->first, Channel{edge->capacity, 0});
  }
}

bool khvaevskiy::FlowSolver::find_path(const std::string & source, const std::string & sink, Parents & parents,
                                       bool residual) const
{
  parents.clear();
  parents.insert(source, source);
  List< std::string > pending;
  pending.push_back(source);
  while (!pending.empty())
  {
    std::string current = pending.front();
    pending.pop_front();
    for (const auto & entry: graph_.at(current))
    {
      const auto & channel = entry.second;
      const auto available = residual ? channel.capacity - channel.flow : channel.flow;
      if (available <= 0 || parents.contains(entry.first))
      {
        continue;
      }
      parents.insert(entry.first, current);
      if (entry.first == sink)
      {
        return true;
      }
      pending.push_back(entry.first);
    }
  }
  return false;
}

khvaevskiy::Capacity khvaevskiy::FlowSolver::path_capacity(const std::string & source, const std::string & sink,
                                                           const Parents & parents, Capacity limit, bool residual) const
{
  for (std::string vertex = sink; vertex != source; vertex = parents.at(vertex))
  {
    const auto & channel = graph_.at(parents.at(vertex)).at(vertex);
    const auto available = residual ? channel.capacity - channel.flow : channel.flow;
    if (available < limit)
    {
      limit = available;
    }
  }
  return limit;
}

void khvaevskiy::FlowSolver::augment(const std::string & source, const std::string & sink, const Parents & parents,
                                     Capacity amount)
{
  for (std::string vertex = sink; vertex != source; vertex = parents.at(vertex))
  {
    const auto & previous = parents.at(vertex);
    graph_.at(previous).at(vertex).flow += amount;
    graph_.at(vertex).at(previous).flow -= amount;
  }
}

khvaevskiy::List< khvaevskiy::FlowPath > khvaevskiy::FlowSolver::decompose(const std::string & source,
                                                                           const std::string & sink, Capacity total)
{
  List< FlowPath > paths;
  Parents parents;
  Capacity decomposed = 0;
  while (decomposed < total && find_path(source, sink, parents, false))
  {
    const auto amount = path_capacity(source, sink, parents, total - decomposed, false);
    FlowPath path{List< std::string >(), amount};
    for (std::string vertex = sink; vertex != source; vertex = parents.at(vertex))
    {
      path.devices.push_front(vertex);
    }
    path.devices.push_front(source);
    paths.push_back(std::move(path));
    augment(source, sink, parents, -amount);
    decomposed += amount;
  }
  if (decomposed != total)
  {
    throw std::logic_error("Flow decomposition invariant failed");
  }
  return paths;
}

khvaevskiy::RouteResult khvaevskiy::FlowSolver::solve(const std::string & source, const std::string & sink,
                                                      Capacity required)
{
  validate_capacity(required);
  if (source == sink || !graph_.contains(source) || !graph_.contains(sink))
  {
    throw std::invalid_argument("Invalid flow endpoints");
  }
  // A solver can be reused; every request starts from zero signed flow.
  for (auto & vertex: graph_)
  {
    for (auto & entry: vertex.second)
    {
      entry.second.flow = 0;
    }
  }
  Parents parents;
  Capacity total = 0;
  while (total < required && find_path(source, sink, parents, true))
  {
    const auto amount = path_capacity(source, sink, parents, required - total, true);
    // f(u,v) = -f(v,u), residual(u,v) = capacity(u,v) - f(u,v).
    augment(source, sink, parents, amount);
    total += amount;
  }
  if (total < required)
  {
    return {false, required, total, List< FlowPath >()};
  }
  return {true, required, total, decompose(source, sink, total)};
}
