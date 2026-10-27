#ifndef KHVAEVSKIY_NETWORK_HPP
#define KHVAEVSKIY_NETWORK_HPP

#include <hash_table.hpp>
#include <list.hpp>
#include "network_types.hpp"

namespace khvaevskiy
{
  class Network
  {
  public:
    using const_iterator = HashTable< std::string, Device >::const_iterator;
    using edge_iterator = List< Edge >::const_iterator;

    Network() = default;
    Network(const Network &) = default;
    Network(Network &&) = default;
    Network & operator=(Network rhs) noexcept;
    void swap(Network & rhs) noexcept;

    void add_device(const std::string & name, const std::string & ip);
    void remove_device(const std::string & name);
    void rename_device(const std::string & old_name, const std::string & new_name);
    void set_ip(const std::string & name, const std::string & ip);
    void connect(const std::string & first, const std::string & second, Capacity capacity);
    void disconnect(const std::string & first, const std::string & second);
    DeviceInfo device(const std::string & name) const;
    List< Neighbor > neighbors(const std::string & name) const;
    RouteResult route(const std::string & source, const std::string & sink, Capacity required) const;

    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    edge_iterator edges_begin() const noexcept;
    edge_iterator edges_end() const noexcept;
    std::size_t device_count() const noexcept;
    std::size_t edge_count() const noexcept;
    void clear() noexcept;

  private:
    HashTable< std::string, Device > devices_;
    List< Edge > edges_;

    void require_device(const std::string & name) const;
    void require_free_ip(const std::string & ip, const std::string & except = "") const;
  };
} // namespace khvaevskiy

#endif
