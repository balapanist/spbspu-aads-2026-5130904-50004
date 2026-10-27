#include "test-support.hpp"
#include <cstdlib>
#include <new>
#include "output.hpp"

namespace khvaevskiy
{
  // Test-only allocation fault injection. Disabled outside the operation under test.
  long allocations_before_failure = -1;

  class AllocationFailure
  {
  public:
    explicit AllocationFailure(long count) noexcept
    {
      allocations_before_failure = count;
    }

    ~AllocationFailure()
    {
      allocations_before_failure = -1;
    }

    AllocationFailure(const AllocationFailure &) = delete;
    AllocationFailure & operator=(const AllocationFailure &) = delete;
  };

  std::string snapshot(const Network & network)
  {
    std::map< std::string, std::string > devices;
    std::map< std::pair< std::string, std::string >, Capacity > edges;
    for (const auto & entry: network)
    {
      devices[entry.first] = entry.second.ip;
    }
    for (auto edge = network.edges_begin(); edge != network.edges_end(); ++edge)
    {
      edges[{edge->first, edge->second}] = edge->capacity;
    }
    std::ostringstream output;
    for (const auto & device: devices)
    {
      output << device.first << ' ' << device.second << '\n';
    }
    for (const auto & edge: edges)
    {
      output << edge.first.first << ' ' << edge.first.second << ' ' << edge.second << '\n';
    }
    return output.str();
  }

  template< class Operation >
  void check_allocation_failures(const Network & original, Operation operation)
  {
    const auto before = snapshot(original);
    std::size_t failures = 0;
    bool success = false;
    for (long allocation = 0; allocation < 2000 && !success; ++allocation)
    {
      Network network(original);
      bool failed = false;
      {
        AllocationFailure failure(allocation);
        try
        {
          operation(network);
        }
        catch (const std::bad_alloc &)
        {
          failed = true;
        }
      }
      if (failed)
      {
        ++failures;
        BOOST_CHECK(snapshot(network) == before);
      }
      else
      {
        success = true;
      }
    }
    BOOST_CHECK(success);
    BOOST_CHECK(failures > 0);
  }

  struct ThrowingValue
  {
    static int alive;
    static int copies_left;
    int value;

    explicit ThrowingValue(int number = 0) : value(number)
    {
      ++alive;
    }

    ThrowingValue(const ThrowingValue & rhs) : value(rhs.value)
    {
      if (copies_left == 0)
      {
        throw std::runtime_error("copy failure");
      }
      if (copies_left > 0)
      {
        --copies_left;
      }
      ++alive;
    }

    ThrowingValue(ThrowingValue && rhs) noexcept : value(rhs.value)
    {
      ++alive;
    }

    ~ThrowingValue()
    {
      --alive;
    }
  };

  int ThrowingValue::alive = 0;
  int ThrowingValue::copies_left = -1;
} // namespace khvaevskiy

// Replacement allocation functions are confined to the test executable.
void * operator new(std::size_t size)
{
  if (khvaevskiy::allocations_before_failure == 0)
  {
    khvaevskiy::allocations_before_failure = -1;
    throw std::bad_alloc();
  }
  if (khvaevskiy::allocations_before_failure > 0)
  {
    --khvaevskiy::allocations_before_failure;
  }
  void * memory = std::malloc(size ? size : 1);
  if (!memory)
  {
    throw std::bad_alloc();
  }
  return memory;
}

void * operator new[](std::size_t size)
{
  return ::operator new(size);
}

void operator delete(void * memory) noexcept
{
  std::free(memory);
}

void operator delete[](void * memory) noexcept
{
  ::operator delete(memory);
}

void operator delete(void * memory, std::size_t) noexcept
{
  ::operator delete(memory);
}

void operator delete[](void * memory, std::size_t) noexcept
{
  ::operator delete(memory);
}

// MSVC debug strings allocate iterator proxies inside noexcept constructors.
// Injecting a failure there terminates the CRT itself. Run this case in Release.
#if !defined(_MSC_VER) || _ITERATOR_DEBUG_LEVEL == 0
BOOST_AUTO_TEST_CASE(graph_strong_guarantee_under_allocation_failure)
{
  auto network = khvaevskiy::make_network(4);
  network.connect("n0", "n1", 3);
  network.connect("n1", "n3", 3);
  network.connect("n0", "n2", 2);
  network.connect("n2", "n3", 2);
  khvaevskiy::check_allocation_failures(network, [](khvaevskiy::Network & value)
                                        { value.rename_device("n1", "renamed_device_with_a_long_name"); });
  khvaevskiy::check_allocation_failures(network, [](khvaevskiy::Network & value) { value.remove_device("n1"); });
  khvaevskiy::check_allocation_failures(network, [](khvaevskiy::Network & value) { value.disconnect("n0", "n1"); });
  khvaevskiy::check_allocation_failures(network, [](khvaevskiy::Network & value)
                                        { value.add_device("new_device_with_a_long_name", "10.1.2.3"); });
  khvaevskiy::check_allocation_failures(network, [](khvaevskiy::Network & value) { value.connect("n0", "n3", 1); });
  khvaevskiy::check_allocation_failures(network,
                                        [](khvaevskiy::Network & value)
                                        {
                                          const auto result = value.route("n0", "n3", 5);
                                          if (!result.possible)
                                          {
                                            throw std::logic_error("Unexpected infeasibility");
                                          }
                                        });
  const auto replacement = khvaevskiy::make_network(2);
  khvaevskiy::check_allocation_failures(network, [&](khvaevskiy::Network & value) { value = replacement; });
}
#endif

BOOST_AUTO_TEST_CASE(container_copy_failure_does_not_leak_or_overwrite)
{
  using khvaevskiy::ThrowingValue;
  {
    khvaevskiy::HashTable< int, ThrowingValue > source;
    source.insert(1, ThrowingValue(10));
    source.insert(2, ThrowingValue(20));
    khvaevskiy::HashTable< int, ThrowingValue > target;
    target.insert(9, ThrowingValue(90));
    const int alive = ThrowingValue::alive;
    ThrowingValue::copies_left = 1;
    BOOST_CHECK_THROW(target = source, std::runtime_error);
    ThrowingValue::copies_left = -1;
    BOOST_REQUIRE(target.size() == 1);
    BOOST_CHECK(target.at(9).value == 90);
    BOOST_CHECK(ThrowingValue::alive == alive);

    khvaevskiy::List< ThrowingValue > list;
    list.push_back(ThrowingValue(1));
    list.push_back(ThrowingValue(2));
    khvaevskiy::List< ThrowingValue > copy;
    copy.push_back(ThrowingValue(99));
    const int list_alive = ThrowingValue::alive;
    ThrowingValue::copies_left = 1;
    BOOST_CHECK_THROW(copy = list, std::runtime_error);
    ThrowingValue::copies_left = -1;
    BOOST_REQUIRE(copy.size() == 1);
    BOOST_CHECK(copy.front().value == 99);
    BOOST_CHECK(ThrowingValue::alive == list_alive);
  }
  BOOST_CHECK(ThrowingValue::alive == 0);
}
