#ifndef KHVAEVSKIY_HASH_TABLE_HPP
#define KHVAEVSKIY_HASH_TABLE_HPP

#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace khvaevskiy
{
  template< class Key, class Value, class Hash = std::hash< Key >, class Equal = std::equal_to< Key > >
  class HashTable
  {
  public:
    using value_type = std::pair< const Key, Value >;

  private:
    struct Slot
    {
      std::unique_ptr< value_type > entry;
      bool erased = false;
    };

    template< bool Const >
    class Iterator
    {
      friend class HashTable;
      template< bool >
      friend class Iterator;
      using Owner = typename std::conditional< Const, const HashTable, HashTable >::type;
      Owner * owner_;
      std::size_t index_;

      Iterator(Owner * owner, std::size_t index) noexcept : owner_(owner), index_(index)
      {
      }

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = typename HashTable::value_type;
      using difference_type = std::ptrdiff_t;
      using reference = typename std::conditional< Const, const value_type &, value_type & >::type;
      using pointer = typename std::conditional< Const, const value_type *, value_type * >::type;

      Iterator() noexcept : owner_(nullptr), index_(0)
      {
      }

      template< bool C = Const, typename std::enable_if< C, int >::type = 0 >
      Iterator(const Iterator< false > & rhs) noexcept : owner_(rhs.owner_), index_(rhs.index_)
      {
      }

      reference operator*() const noexcept
      {
        return *owner_->slots_[index_].entry;
      }
      pointer operator->() const noexcept
      {
        return owner_->slots_[index_].entry.get();
      }

      Iterator & operator++() noexcept
      {
        ++index_;
        while (index_ < owner_->capacity_ && !owner_->slots_[index_].entry)
        {
          ++index_;
        }
        return *this;
      }

      Iterator operator++(int) noexcept
      {
        Iterator old(*this);
        ++(*this);
        return old;
      }

      template< bool C >
      bool operator==(const Iterator< C > & rhs) const noexcept
      {
        return owner_ == rhs.owner_ && index_ == rhs.index_;
      }

      template< bool C >
      bool operator!=(const Iterator< C > & rhs) const noexcept
      {
        return !(*this == rhs);
      }
    };

  public:
    using iterator = Iterator< false >;
    using const_iterator = Iterator< true >;

    explicit HashTable(std::size_t capacity = 8, Hash hash = Hash(), Equal equal = Equal())
        : slots_(nullptr), capacity_(0), size_(0), hash_(std::move(hash)), equal_(std::move(equal))
    {
      static_assert(noexcept(std::swap(hash_, hash_)), "Hash must support nonthrowing swap");
      static_assert(noexcept(std::swap(equal_, equal_)), "Equal must support nonthrowing swap");
      rehash(capacity);
    }

    HashTable(const HashTable & rhs) : HashTable(rhs.capacity_, rhs.hash_, rhs.equal_)
    {
      for (const auto & entry: rhs)
      {
        insert(entry.first, entry.second);
      }
    }

    HashTable(HashTable && rhs) : HashTable(8, rhs.hash_, rhs.equal_)
    {
      swap(rhs);
    }

    HashTable & operator=(HashTable rhs) noexcept
    {
      swap(rhs);
      return *this;
    }

    void swap(HashTable & rhs) noexcept
    {
      slots_.swap(rhs.slots_);
      std::swap(capacity_, rhs.capacity_);
      std::swap(size_, rhs.size_);
      std::swap(hash_, rhs.hash_);
      std::swap(equal_, rhs.equal_);
    }

    std::size_t size() const noexcept
    {
      return size_;
    }
    std::size_t bucket_count() const noexcept
    {
      return capacity_;
    }
    bool empty() const noexcept
    {
      return size_ == 0;
    }
    iterator begin() noexcept
    {
      return iterator(this, first_occupied());
    }
    iterator end() noexcept
    {
      return iterator(this, capacity_);
    }
    const_iterator begin() const noexcept
    {
      return const_iterator(this, first_occupied());
    }
    const_iterator end() const noexcept
    {
      return const_iterator(this, capacity_);
    }
    const_iterator cbegin() const noexcept
    {
      return begin();
    }
    const_iterator cend() const noexcept
    {
      return end();
    }

    iterator find(const Key & key)
    {
      return iterator(this, locate(key));
    }
    const_iterator find(const Key & key) const
    {
      return const_iterator(this, locate(key));
    }
    bool contains(const Key & key) const
    {
      return find(key) != end();
    }

    Value & at(const Key & key)
    {
      auto found = find(key);
      if (found == end())
      {
        throw std::out_of_range("HashTable: key not found");
      }
      return found->second;
    }

    const Value & at(const Key & key) const
    {
      auto found = find(key);
      if (found == end())
      {
        throw std::out_of_range("HashTable: key not found");
      }
      return found->second;
    }

    std::pair< iterator, bool > insert(Key key, Value value)
    {
      auto found = find(key);
      if (found != end())
      {
        return {found, false};
      }
      const auto hash = hash_(key);
      auto entry = std::make_unique< value_type >(std::move(key), std::move(value));
      if (size_ >= capacity_ / 2)
      {
        if (capacity_ > std::numeric_limits< std::size_t >::max() / 2)
        {
          throw std::length_error("HashTable capacity overflow");
        }
        rehash(capacity_ * 2);
      }
      auto index = hash % capacity_;
      while (slots_[index].entry)
      {
        index = next_index(index);
      }
      slots_[index].entry = std::move(entry);
      slots_[index].erased = false;
      ++size_;
      return {iterator(this, index), true};
    }

    Value & operator[](const Key & key)
    {
      auto found = find(key);
      if (found != end())
      {
        return found->second;
      }
      return insert(key, Value()).first->second;
    }

    bool erase(const Key & key)
    {
      const auto index = locate(key);
      if (index == capacity_)
      {
        return false;
      }
      slots_[index].entry.reset();
      slots_[index].erased = true;
      --size_;
      return true;
    }

    void clear() noexcept
    {
      for (std::size_t i = 0; i < capacity_; ++i)
      {
        slots_[i].entry.reset();
        slots_[i].erased = false;
      }
      size_ = 0;
    }

    void rehash(std::size_t capacity)
    {
      if (size_ > std::numeric_limits< std::size_t >::max() / 2)
      {
        throw std::length_error("HashTable capacity overflow");
      }
      if (capacity < size_ * 2)
      {
        capacity = size_ * 2;
      }
      if (capacity < 8)
      {
        capacity = 8;
      }
      auto next = std::make_unique< Slot[] >(capacity);
      auto destination = std::make_unique< std::size_t[] >(capacity_);
      // Plan every move before touching the old storage. Hashing may throw.
      for (std::size_t i = 0; i < capacity_; ++i)
      {
        if (slots_[i].entry)
        {
          auto index = hash_(slots_[i].entry->first) % capacity;
          while (next[index].erased)
          {
            index = index + 1 == capacity ? 0 : index + 1;
          }
          next[index].erased = true;
          destination[i] = index;
        }
      }
      for (std::size_t i = 0; i < capacity_; ++i)
      {
        if (slots_[i].entry)
        {
          next[destination[i]].entry = std::move(slots_[i].entry);
          next[destination[i]].erased = false;
        }
      }
      slots_.swap(next);
      capacity_ = capacity;
    }

  private:
    std::unique_ptr< Slot[] > slots_;
    std::size_t capacity_;
    std::size_t size_;
    Hash hash_;
    Equal equal_;

    std::size_t next_index(std::size_t index) const noexcept
    {
      return index + 1 == capacity_ ? 0 : index + 1;
    }

    std::size_t first_occupied() const noexcept
    {
      std::size_t index = 0;
      while (index < capacity_ && !slots_[index].entry)
      {
        ++index;
      }
      return index;
    }

    std::size_t locate(const Key & key) const
    {
      auto index = hash_(key) % capacity_;
      for (std::size_t checked = 0; checked < capacity_; ++checked)
      {
        const auto & slot = slots_[index];
        if (slot.entry && equal_(slot.entry->first, key))
        {
          return index;
        }
        if (!slot.entry && !slot.erased)
        {
          return capacity_;
        }
        index = next_index(index);
      }
      return capacity_;
    }
  };
} // namespace khvaevskiy

#endif
