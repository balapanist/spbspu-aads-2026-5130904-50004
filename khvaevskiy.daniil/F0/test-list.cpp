#include "test-support.hpp"
#include <list.hpp>

BOOST_AUTO_TEST_CASE(list_iteration_copy_and_move)
{
  khvaevskiy::List< std::string > list;
  BOOST_CHECK(list.empty());
  BOOST_CHECK(list.begin() == list.end());
  BOOST_CHECK_THROW(list.front(), std::out_of_range);
  BOOST_CHECK_THROW(list.pop_front(), std::out_of_range);
  list.push_back(std::string("b"));
  list.push_front(std::string("a"));
  list.push_back(std::string("c"));
  BOOST_REQUIRE(list.size() == 3);
  auto it = list.begin();
  BOOST_CHECK(*it++ == "a");
  BOOST_CHECK(*it == "b");
  *it = "B";
  const auto & view = list;
  khvaevskiy::List< std::string >::const_iterator ci = it;
  BOOST_CHECK(ci == it && it == ci);
  BOOST_CHECK(std::distance(view.begin(), view.end()) == 3);
  auto copy = list;
  copy.front() = "copy";
  BOOST_CHECK(list.front() == "a");
  auto moved = std::move(copy);
  BOOST_CHECK(copy.empty());
  copy.push_back(std::string("reused"));
  BOOST_CHECK(copy.front() == "reused");
  list = moved;
  list = list;
  BOOST_CHECK(list.front() == "copy");
  list = std::move(moved);
  BOOST_CHECK(moved.empty());
  list.pop_front();
  BOOST_CHECK(list.front() == "B");
  list.clear();
  list.push_back(std::string("last"));
  BOOST_CHECK(list.size() == 1);
}

BOOST_AUTO_TEST_CASE(list_long_chain_destruction_is_iterative)
{
  khvaevskiy::List< int > list;
  for (int i = 0; i < 100000; ++i)
  {
    list.push_back(i);
  }
  BOOST_CHECK(list.size() == 100000);
  list.clear();
  BOOST_CHECK(list.empty());
}

BOOST_AUTO_TEST_CASE(hash_tombstones_and_const_interface)
{
  khvaevskiy::HashTable< int, int, khvaevskiy::CollisionHash > table;
  for (int i = 0; i < 4; ++i)
  {
    table.insert(i, i);
  }
  table.erase(0);
  BOOST_CHECK(table.at(3) == 3);
  BOOST_CHECK(!table.insert(3, 99).second);
  table.insert(7, 7);
  BOOST_CHECK(table.at(7) == 7);
  for (int i = 0; i < 100; ++i)
  {
    table.insert(100 + i, i);
    table.erase(100 + i);
  }
  BOOST_CHECK(!table.contains(999));
  table.insert(999, 42);
  BOOST_CHECK(table.at(999) == 42);
}
