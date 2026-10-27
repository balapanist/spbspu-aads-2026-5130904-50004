#ifndef KHVAEVSKIY_TEST_SUPPORT_HPP
#define KHVAEVSKIY_TEST_SUPPORT_HPP

#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <hash_table.hpp>
#include "console.hpp"

namespace khvaevskiy
{
  template< class Error = std::invalid_argument, class Function >
  void throws(Function action)
  {
    BOOST_CHECK_THROW(action(), Error);
  }

  struct CollisionHash
  {
    std::size_t operator()(int) const
    {
      return 0;
    }
  };

  struct ThrowHash
  {
    std::shared_ptr< bool > fail;
    std::size_t operator()(int key) const
    {
      if (*fail && key == 2)
      {
        throw std::runtime_error("hash failure");
      }
      return static_cast< std::size_t >(key);
    }
  };

  Network make_network(int count);
  void verify_paths(const Network & network, const RouteResult & result, const std::string & source,
                    const std::string & sink);
} // namespace khvaevskiy

#endif
