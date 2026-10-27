#ifndef KHVAEVSKIY_LIST_HPP
#define KHVAEVSKIY_LIST_HPP

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace khvaevskiy
{
  template< class T >
  class List
  {
    struct Node
    {
      T value;
      std::unique_ptr< Node > next;

      template< class U >
      explicit Node(U && item) : value(std::forward< U >(item)), next(nullptr)
      {
      }
    };

    template< bool Const >
    class Iterator
    {
      friend class List;
      template< bool >
      friend class Iterator;
      using NodePointer = typename std::conditional< Const, const Node *, Node * >::type;
      NodePointer node_;

      explicit Iterator(NodePointer node) noexcept : node_(node)
      {
      }

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = T;
      using difference_type = std::ptrdiff_t;
      using reference = typename std::conditional< Const, const T &, T & >::type;
      using pointer = typename std::conditional< Const, const T *, T * >::type;

      Iterator() noexcept : node_(nullptr)
      {
      }

      template< bool C = Const, typename std::enable_if< C, int >::type = 0 >
      Iterator(const Iterator< false > & rhs) noexcept : node_(rhs.node_)
      {
      }

      reference operator*() const noexcept
      {
        return node_->value;
      }

      pointer operator->() const noexcept
      {
        return std::addressof(node_->value);
      }

      Iterator & operator++() noexcept
      {
        node_ = node_->next.get();
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
        return node_ == rhs.node_;
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

    List() noexcept : head_(nullptr), tail_(nullptr), size_(0)
    {
    }

    List(const List & rhs) : List()
    {
      List temporary;
      for (const T & value: rhs)
      {
        temporary.push_back(value);
      }
      swap(temporary);
    }

    List(List && rhs) noexcept : List()
    {
      swap(rhs);
    }

    ~List()
    {
      clear();
    }

    List & operator=(List rhs) noexcept
    {
      swap(rhs);
      return *this;
    }

    void swap(List & rhs) noexcept
    {
      head_.swap(rhs.head_);
      std::swap(tail_, rhs.tail_);
      std::swap(size_, rhs.size_);
    }

    template< class U >
    void push_back(U && value)
    {
      auto node = std::make_unique< Node >(std::forward< U >(value));
      Node * last = node.get();
      if (tail_)
      {
        tail_->next = std::move(node);
      }
      else
      {
        head_ = std::move(node);
      }
      tail_ = last;
      ++size_;
    }

    template< class U >
    void push_front(U && value)
    {
      auto node = std::make_unique< Node >(std::forward< U >(value));
      if (!head_)
      {
        tail_ = node.get();
      }
      node->next = std::move(head_);
      head_ = std::move(node);
      ++size_;
    }

    T & front()
    {
      if (empty())
      {
        throw std::out_of_range("Empty list");
      }
      return head_->value;
    }

    const T & front() const
    {
      if (empty())
      {
        throw std::out_of_range("Empty list");
      }
      return head_->value;
    }

    void pop_front()
    {
      if (empty())
      {
        throw std::out_of_range("Empty list");
      }
      auto old = std::move(head_);
      head_ = std::move(old->next);
      --size_;
      if (!head_)
      {
        tail_ = nullptr;
      }
    }

    void clear() noexcept
    {
      // Detach each next pointer: destruction does not recurse through the chain.
      while (head_)
      {
        auto old = std::move(head_);
        head_ = std::move(old->next);
      }
      tail_ = nullptr;
      size_ = 0;
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }
    std::size_t size() const noexcept
    {
      return size_;
    }
    iterator begin() noexcept
    {
      return iterator(head_.get());
    }
    iterator end() noexcept
    {
      return iterator(nullptr);
    }
    const_iterator begin() const noexcept
    {
      return const_iterator(head_.get());
    }
    const_iterator end() const noexcept
    {
      return const_iterator(nullptr);
    }
    const_iterator cbegin() const noexcept
    {
      return begin();
    }
    const_iterator cend() const noexcept
    {
      return end();
    }

  private:
    std::unique_ptr< Node > head_;
    Node * tail_;
    std::size_t size_;
  };
} // namespace khvaevskiy

#endif
