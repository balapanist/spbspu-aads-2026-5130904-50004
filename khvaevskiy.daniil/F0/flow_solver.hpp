#ifndef KHVAEVSKIY_FLOW_SOLVER_HPP
#define KHVAEVSKIY_FLOW_SOLVER_HPP

#include <hash_table.hpp>
#include "network.hpp"

namespace khvaevskiy
{
  class FlowSolver
  {
  public:
    explicit FlowSolver(const Network & network);
    RouteResult solve(const std::string & source, const std::string & sink, Capacity required);

  private:
    struct Channel
    {
      Capacity capacity;
      Capacity flow;
    };

    using Graph = HashTable< std::string, HashTable< std::string, Channel > >;
    using Parents = HashTable< std::string, std::string >;
    Graph graph_;

    bool find_path(const std::string & source, const std::string & sink, Parents & parents, bool residual) const;
    Capacity path_capacity(const std::string & source, const std::string & sink, const Parents & parents,
                           Capacity limit, bool residual) const;
    void augment(const std::string & source, const std::string & sink, const Parents & parents, Capacity amount);
    List< FlowPath > decompose(const std::string & source, const std::string & sink, Capacity total);
  };
} // namespace khvaevskiy

#endif
