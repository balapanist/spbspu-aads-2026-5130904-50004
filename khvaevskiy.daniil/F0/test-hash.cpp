#include "test-support.hpp"

namespace khvaevskiy
{
  void hash_collisions()
  {
    khvaevskiy::HashTable< int, std::string, CollisionHash > table(1);
    BOOST_REQUIRE_MESSAGE(table.begin() == table.end(), "Empty iterators");
    for (int i = 0; i < 200; ++i)
    {
      BOOST_REQUIRE_MESSAGE(table.insert(i, std::to_string(i)).second, "Insert");
    }
    BOOST_REQUIRE_MESSAGE(table.size() == 200 && table.bucket_count() >= 200, "Growth");
    BOOST_REQUIRE_MESSAGE(!table.insert(10, "replacement").second && table.at(10) == "10",
                          "Duplicate insert preserves value");
    BOOST_REQUIRE_MESSAGE(table.erase(0) && table.erase(99) && table.erase(199), "Erase chain head/middle/tail");
    BOOST_REQUIRE_MESSAGE(!table.erase(999), "Erase missing");
    for (int i = 1; i < 199; ++i)
    {
      if (i != 99)
      {
        BOOST_REQUIRE_MESSAGE(table.at(i) == std::to_string(i), "Collision lookup");
      }
    }
    throws< std::out_of_range >([&] { (void)table.at(999); });
    table.clear();
    BOOST_REQUIRE_MESSAGE(table.empty(), "Clear");
    table[42] = "new";
    BOOST_REQUIRE_MESSAGE(table.at(42) == "new", "Reuse after clear");
  }

  void hash_iterators_copy_move()
  {
    khvaevskiy::HashTable< int, int > table;
    for (int i = 0; i < 50; ++i)
    {
      table.insert(i, i);
    }
    std::set< int > keys;
    for (auto it = table.begin(); it != table.end(); it++)
    {
      keys.insert(it->first);
      it->second += 1;
    }
    BOOST_REQUIRE_MESSAGE(keys.size() == table.size(), "Iterator visits every bucket");
    const auto & view = table;
    typename khvaevskiy::HashTable< int, int >::const_iterator ci = table.begin();
    BOOST_REQUIRE_MESSAGE(ci == table.begin() && table.begin() == ci, "Mixed iterator equality");
    static_assert(std::is_const< typename std::remove_reference< decltype((ci->second)) >::type >::value,
                  "Const value");
    static_assert(std::is_const< typename std::remove_reference< decltype((table.begin()->first)) >::type >::value,
                  "Const key");
    int sum = 0;
    for (auto it = view.cbegin(); it != view.cend(); ++it)
    {
      sum += it->second;
    }
    BOOST_REQUIRE_MESSAGE(sum == 1275, "Const traversal");
    BOOST_REQUIRE_MESSAGE(std::distance(view.begin(), view.end()) == 50, "Standard iterator traits");
    auto copy = table;
    copy.at(0) = -1;
    BOOST_REQUIRE_MESSAGE(table.at(0) == 1, "Copy owns independent values");
    khvaevskiy::HashTable< int, int > assigned;
    assigned = copy;
    assigned = assigned;
    BOOST_REQUIRE_MESSAGE(assigned.at(0) == -1, "Copy/self assignment");
    auto moved = std::move(copy);
    BOOST_REQUIRE_MESSAGE(moved.size() == 50 && copy.empty(), "Move construction");
    copy.insert(100, 5);
    BOOST_REQUIRE_MESSAGE(copy.at(100) == 5, "Moved-from reuse");
    assigned = std::move(moved);
    BOOST_REQUIRE_MESSAGE(assigned.size() == 50 && moved.empty(), "Move assignment");
    table.rehash(2);
    BOOST_REQUIRE_MESSAGE(table.size() == 50 && table.at(49) == 50, "Explicit rehash");
    khvaevskiy::HashTable< int, std::unique_ptr< int > > owning;
    owning.insert(1, std::make_unique< int >(7));
    owning.rehash(20);
    BOOST_REQUIRE_MESSAGE(*owning.at(1) == 7, "Move-only values survive rehash");
  }

  void hash_exception_safety()
  {
    auto fail = std::make_shared< bool >(false);
    khvaevskiy::HashTable< int, int, ThrowHash > table(8, ThrowHash{fail});
    table.insert(1, 11);
    table.insert(2, 22);
    *fail = true;
    throws< std::runtime_error >([&] { table.rehash(100); });
    *fail = false;
    BOOST_REQUIRE_MESSAGE(table.bucket_count() == 8 && table.size() == 2 && table.at(1) == 11 && table.at(2) == 22,
                          "Throwing hash must not partially move nodes");
  }
} // namespace khvaevskiy

BOOST_AUTO_TEST_CASE(hash_collisions)
{
  khvaevskiy::hash_collisions();
}

BOOST_AUTO_TEST_CASE(hash_iterators_copy_move)
{
  khvaevskiy::hash_iterators_copy_move();
}

BOOST_AUTO_TEST_CASE(hash_exception_safety)
{
  khvaevskiy::hash_exception_safety();
}
