#include "network.hpp"
#include <stdexcept>
#include <utility>
#include "validation.hpp"

khvaevskiy::Network & khvaevskiy::Network::operator=(Network rhs) noexcept
{
  swap(rhs);
  return *this;
}

void khvaevskiy::Network::swap(Network & rhs) noexcept
{
  devices_.swap(rhs.devices_);
  edges_.swap(rhs.edges_);
}

void khvaevskiy::Network::require_device(const std::string & name) const
{
  if (!devices_.contains(name))
  {
    throw std::invalid_argument("Unknown device: " + name);
  }
}

void khvaevskiy::Network::require_free_ip(const std::string & ip, const std::string & except) const
{
  validate_ip(ip);
  for (const auto & entry: devices_)
  {
    if (entry.first != except && entry.second.ip == ip)
    {
      throw std::invalid_argument("IP address is already in use");
    }
  }
}

void khvaevskiy::Network::add_device(const std::string & name, const std::string & ip)
{
  validate_name(name);
  if (devices_.contains(name))
  {
    throw std::invalid_argument("Device already exists: " + name);
  }
  require_free_ip(ip);
  devices_.insert(name, Device{ip});
}

void khvaevskiy::Network::remove_device(const std::string & name)
{
  require_device(name);
  Network next(*this);
  List< Edge > remaining;
  for (const Edge & edge: next.edges_)
  {
    if (edge.first != name && edge.second != name)
    {
      remaining.push_back(edge);
    }
  }
  next.edges_.swap(remaining);
  next.devices_.erase(name);
  swap(next);
}

void khvaevskiy::Network::rename_device(const std::string & old_name, const std::string & new_name)
{
  require_device(old_name);
  validate_name(new_name);
  if (old_name == new_name)
  {
    return;
  }
  if (devices_.contains(new_name))
  {
    throw std::invalid_argument("Device already exists: " + new_name);
  }
  Network next(*this);
  next.devices_.insert(new_name, next.devices_.at(old_name));
  next.devices_.erase(old_name);
  for (auto & edge: next.edges_)
  {
    if (edge.first == old_name)
    {
      edge.first = new_name;
    }
    if (edge.second == old_name)
    {
      edge.second = new_name;
    }
    if (edge.first > edge.second)
    {
      edge.first.swap(edge.second);
    }
  }
  swap(next);
}

void khvaevskiy::Network::set_ip(const std::string & name, const std::string & ip)
{
  require_device(name);
  require_free_ip(ip, name);
  std::string next(ip);
  devices_.at(name).ip.swap(next);
}

void khvaevskiy::Network::connect(const std::string & first, const std::string & second, Capacity capacity)
{
  require_device(first);
  require_device(second);
  validate_capacity(capacity);
  if (first == second)
  {
    throw std::invalid_argument("Self-connections are not allowed");
  }
  for (auto & edge: edges_)
  {
    if (same_edge(edge, first, second))
    {
      if (edge.capacity > max_capacity - capacity)
      {
        throw std::overflow_error("Combined capacity exceeds supported range");
      }
      edge.capacity += capacity;
      return;
    }
  }
  Edge edge{first, second, capacity};
  if (edge.first > edge.second)
  {
    edge.first.swap(edge.second);
  }
  edges_.push_back(std::move(edge));
}

void khvaevskiy::Network::disconnect(const std::string & first, const std::string & second)
{
  require_device(first);
  require_device(second);
  List< Edge > remaining;
  bool found = false;
  for (const auto & edge: edges_)
  {
    if (same_edge(edge, first, second))
    {
      found = true;
    }
    else
    {
      remaining.push_back(edge);
    }
  }
  if (!found)
  {
    throw std::invalid_argument("Connection does not exist");
  }
  edges_.swap(remaining);
}

khvaevskiy::DeviceInfo khvaevskiy::Network::device(const std::string & name) const
{
  require_device(name);
  std::size_t count = 0;
  for (const auto & edge: edges_)
  {
    if (edge.first == name || edge.second == name)
    {
      ++count;
    }
  }
  return {name, devices_.at(name).ip, count};
}

khvaevskiy::List< khvaevskiy::Neighbor > khvaevskiy::Network::neighbors(const std::string & name) const
{
  require_device(name);
  List< Neighbor > result;
  for (const auto & edge: edges_)
  {
    if (edge.first == name)
    {
      result.push_back(Neighbor{edge.second, edge.capacity});
    }
    else if (edge.second == name)
    {
      result.push_back(Neighbor{edge.first, edge.capacity});
    }
  }
  return result;
}

khvaevskiy::Network::const_iterator khvaevskiy::Network::begin() const noexcept
{
  return devices_.begin();
}

khvaevskiy::Network::const_iterator khvaevskiy::Network::end() const noexcept
{
  return devices_.end();
}

khvaevskiy::Network::edge_iterator khvaevskiy::Network::edges_begin() const noexcept
{
  return edges_.begin();
}

khvaevskiy::Network::edge_iterator khvaevskiy::Network::edges_end() const noexcept
{
  return edges_.end();
}

std::size_t khvaevskiy::Network::device_count() const noexcept
{
  return devices_.size();
}

std::size_t khvaevskiy::Network::edge_count() const noexcept
{
  return edges_.size();
}

void khvaevskiy::Network::clear() noexcept
{
  edges_.clear();
  devices_.clear();
}
