// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009
// Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/*
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
#ifndef __INTERPKERNELHASHTABLE_HXX__
#define __INTERPKERNELHASHTABLE_HXX__


#include <memory>
#include <cstddef>
#include <utility>
#include <vector>
#include <iterator>
#include <algorithm>

namespace INTERP_KERNEL
{
  template<class _Val>
  struct _Hashtable_node
  {
    _Hashtable_node* _M_next;
    _Val _M_val;
  };

  template<class _Val, class _Key, class _HashFcn, class _ExtractKey, 
           class _EqualKey, class _Alloc = std::allocator<_Val> >
  class hashtable;

  template<class _Val, class _Key, class _HashFcn,
           class _ExtractKey, class _EqualKey, class _Alloc>
  struct _Hashtable_iterator;

  template<class _Val, class _Key, class _HashFcn,
           class _ExtractKey, class _EqualKey, class _Alloc>
  struct _Hashtable_const_iterator;

  template<class _Val, class _Key, class _HashFcn,
           class _ExtractKey, class _EqualKey, class _Alloc>
  struct _Hashtable_iterator
  {
    using _Hashtable = hashtable<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;
    using iterator = _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;
    using const_iterator = _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;
    using _Node = _Hashtable_node<_Val>;
    using iterator_category = std::forward_iterator_tag;
    using value_type = _Val;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using reference = _Val &;
    using pointer = _Val *;
      
    _Node* _M_cur;
    _Hashtable* _M_ht;

    _Hashtable_iterator(_Node* __n, _Hashtable* __tab)
      : _M_cur(__n), _M_ht(__tab) { }

    _Hashtable_iterator() = default;

    reference
    operator*() const
    { return _M_cur->_M_val; }

    pointer
    operator->() const
    { return &(operator*()); }

    iterator&
    operator++();

    iterator
    operator++(int);

    bool
    operator==(const iterator& __it) const
    { return _M_cur == __it._M_cur; }

    bool
    operator!=(const iterator& __it) const
    { return _M_cur != __it._M_cur; }
  };

  template<class _Val, class _Key, class _HashFcn,
           class _ExtractKey, class _EqualKey, class _Alloc>
  struct _Hashtable_const_iterator
  {
    using _Hashtable = hashtable<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;
    using iterator = _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;
    using const_iterator = _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;
    using _Node = _Hashtable_node<_Val>;

    using iterator_category = std::forward_iterator_tag;
    using value_type = _Val;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using reference = const _Val &;
    using pointer = const _Val *;
      
    const _Node* _M_cur;
    const _Hashtable* _M_ht;

    _Hashtable_const_iterator(const _Node* __n, const _Hashtable* __tab)
      : _M_cur(__n), _M_ht(__tab) { }

    _Hashtable_const_iterator() = default;

    _Hashtable_const_iterator(const iterator& __it)
      : _M_cur(__it._M_cur), _M_ht(__it._M_ht) { }

    reference  operator*() const { return _M_cur->_M_val; }

    pointer operator->() const { return &(operator*()); }

    const_iterator& operator++();

    const_iterator operator++(int);

    bool operator==(const const_iterator& __it) const { return _M_cur == __it._M_cur; }

    bool operator!=(const const_iterator& __it) const { return _M_cur != __it._M_cur; }
  };

  // Note: assumes long is at least 32 bits.
  enum { _S_num_primes = 28 };

  static const unsigned long long __stl_prime_list[_S_num_primes] =
    {
      53ull,         97ull,         193ull,       389ull,       769ull,
      1543ull,       3079ull,       6151ull,      12289ull,     24593ull,
      49157ull,      98317ull,      196613ull,    393241ull,    786433ull,
      1572869ull,    3145739ull,    6291469ull,   12582917ull,  25165843ull,
      50331653ull,   100663319ull,  201326611ull, 402653189ull, 805306457ull,
      1610612741ull, 3221225473ull, 4294967291ull
    };

  inline unsigned long long
  __stl_next_prime(unsigned long long __n)
  {
    const unsigned long long* __first = __stl_prime_list;
    const unsigned long long* __last = __stl_prime_list + (int)_S_num_primes;
    const unsigned long long* pos = std::lower_bound(__first, __last, __n);
    return pos == __last ? *(__last - 1) : *pos;
  }

  // Forward declaration of operator==.  
  template<class _Val, class _Key, class _HF, class _Ex,
           class _Eq, class _All>
  class hashtable;

  template<class _Val, class _Key, class _HF, class _Ex,
           class _Eq, class _All>
  bool operator==(const hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>& __ht1,
                  const hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>& __ht2);

  // Hashtables handle allocators a bit differently than other
  // containers do.  If we're using standard-conforming allocators, then
  // a hashtable unconditionally has a member variable to hold its
  // allocator, even if it so happens that all instances of the
  // allocator type are identical.  This is because, for hashtables,
  // this extra storage is negligible.  Additionally, a base class
  // wouldn't serve any other purposes; it wouldn't, for example,
  // simplify the exception-handling code.  
  template<class _Val, class _Key, class _HashFcn,
           class _ExtractKey, class _EqualKey, class _Alloc>
  class hashtable
  {
  public:
    using key_type = _Key;
    using value_type = _Val;
    using hasher = _HashFcn;
    using key_equal = _EqualKey;

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;

    hasher hash_funct() const { return _M_hash; }

    key_equal key_eq() const { return _M_equals; }

  private:
    using _Node = _Hashtable_node<_Val>;

  public:
    using allocator_type = typename _Alloc::template rebind<value_type>::other;
    allocator_type get_allocator() const { return _M_node_allocator; }

  private:
    using _Node_Alloc = typename _Alloc::template rebind<_Node>::other;
    using _Nodeptr_Alloc = typename _Alloc::template rebind<_Node *>::other;
    using _Vector_type = std::vector<_Node *, _Nodeptr_Alloc>;

    _Node_Alloc _M_node_allocator;

    _Node *_M_get_node() { return _M_node_allocator.allocate(1); }

    void _M_put_node(_Node* __p) { _M_node_allocator.deallocate(__p, 1); }

  private:
    hasher                _M_hash;
    key_equal             _M_equals;
    _ExtractKey           _M_get_key;
    _Vector_type          _M_buckets;
    size_type             _M_num_elements;
      
  public:
    using iterator = _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;
    using const_iterator = _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;

    friend struct
    _Hashtable_iterator<_Val, _Key, _HashFcn, _ExtractKey, _EqualKey, _Alloc>;

    friend struct
    _Hashtable_const_iterator<_Val, _Key, _HashFcn, _ExtractKey,
                              _EqualKey, _Alloc>;

  public:
    hashtable(size_type __n, const _HashFcn& __hf,
              const _EqualKey& __eql, const _ExtractKey& __ext,
              const allocator_type& __a = allocator_type())
      : _M_node_allocator(__a), _M_hash(__hf), _M_equals(__eql),
        _M_get_key(__ext), _M_buckets(__a), _M_num_elements(0)
    { _M_initialize_buckets(__n); }

    hashtable(size_type __n, const _HashFcn& __hf,
              const _EqualKey& __eql,
              const allocator_type& __a = allocator_type())
      : _M_node_allocator(__a), _M_hash(__hf), _M_equals(__eql),
        _M_get_key(_ExtractKey()), _M_buckets(__a), _M_num_elements(0)
    { _M_initialize_buckets(__n); }

    hashtable(const hashtable& __ht)
      : _M_node_allocator(__ht.get_allocator()), _M_hash(__ht._M_hash),
        _M_equals(__ht._M_equals), _M_get_key(__ht._M_get_key),
        _M_buckets(__ht.get_allocator()), _M_num_elements(0)
    { _M_copy_from(__ht); }

    hashtable& operator= (const hashtable& __ht)
    {
      if (&__ht != this)
        {
          clear();
          _M_hash = __ht._M_hash;
          _M_equals = __ht._M_equals;
          _M_get_key = __ht._M_get_key;
          _M_copy_from(__ht);
        }
      return *this;
    }

    ~hashtable()
    { clear(); }

    size_type size() const { return _M_num_elements; }

    size_type max_size() const { return size_type(-1); }

    bool empty() const { return size() == 0; }

    void swap(hashtable& __ht) 
    {
      std::swap(_M_hash, __ht._M_hash);
      std::swap(_M_equals, __ht._M_equals);
      std::swap(_M_get_key, __ht._M_get_key);
      _M_buckets.swap(__ht._M_buckets);
      std::swap(_M_num_elements, __ht._M_num_elements);
    }

    iterator begin()
    {
      for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
        if (_M_buckets[__n])
          return iterator(_M_buckets[__n], this);
      return end();
    }

    iterator end() { return iterator(0, this); }

    const_iterator begin() const 
    {
      for (size_type __n = 0; __n < _M_buckets.size(); ++__n)
        if (_M_buckets[__n])
          return const_iterator(_M_buckets[__n], this);
      return end();
    }

    const_iterator end() const { return const_iterator(0, this); }

    template<class _Vl, class _Ky, class _HF, class _Ex, class _Eq, class _Al>
    friend bool operator==(const hashtable<_Vl, _Ky, _HF, _Ex, _Eq, _Al>&,
                           const hashtable<_Vl, _Ky, _HF, _Ex, _Eq, _Al>&);
    
  public:
    size_type bucket_count() const { return _M_buckets.size(); }

    size_type max_bucket_count() const { return __stl_prime_list[(int)_S_num_primes - 1]; }

    size_type elems_in_bucket(size_type __bucket) const 
    {
      size_type __result = 0;
      for (_Node* __n = _M_buckets[__bucket]; __n; __n = __n->_M_next)
        __result += 1;
      return __result;
    }

    std::pair<iterator, bool> insert_unique(const value_type& __obj)
    {
      resize(_M_num_elements + 1);
      return insert_unique_noresize(__obj);
    }

    iterator insert_equal(const value_type& __obj)
    {
      resize(_M_num_elements + 1);
      return insert_equal_noresize(__obj);
    }

    std::pair<iterator, bool> insert_unique_noresize(const value_type& __obj);

    iterator insert_equal_noresize(const value_type& __obj);

    template<class _InputIterator>
    void insert_unique(_InputIterator __f, _InputIterator __l)
    { insert_unique(__f, __l, __iterator_category(__f)); }

    template<class _InputIterator>
    void insert_equal(_InputIterator __f, _InputIterator __l)
    { insert_equal(__f, __l, __iterator_category(__f)); }

    template<class _InputIterator>
    void insert_unique(_InputIterator __f, _InputIterator __l,
                       std::input_iterator_tag)
    {
      for ( ; __f != __l; ++__f)
        insert_unique(*__f);
    }

    template<class _InputIterator>
    void insert_equal(_InputIterator __f, _InputIterator __l,
                      std::input_iterator_tag)
    {
      for ( ; __f != __l; ++__f)
        insert_equal(*__f);
    }
    
    template<class _ForwardIterator>
    void insert_unique(_ForwardIterator __f, _ForwardIterator __l,
                       std::forward_iterator_tag)
    {
      size_type __n = std::distance(__f, __l);
      resize(_M_num_elements + __n);
      for ( ; __n > 0; --__n, ++__f)
        insert_unique_noresize(*__f);
    }

    template<class _ForwardIterator>
    void
    insert_equal(_ForwardIterator __f, _ForwardIterator __l,
                 std::forward_iterator_tag)
    {
      size_type __n = std::distance(__f, __l);
      resize(_M_num_elements + __n);
      for ( ; __n > 0; --__n, ++__f)
        insert_equal_noresize(*__f);
    }

    reference find_or_insert(const value_type& __obj);

    iterator find(const key_type& __key)
    {
      size_type __n = _M_bkt_num_key(__key);
      _Node* __first;
      for (__first = _M_buckets[__n];
           __first && !_M_equals(_M_get_key(__first->_M_val), __key);
           __first = __first->_M_next)
        { }
      return iterator(__first, this);
    }

    const_iterator find(const key_type& __key) const
    {
      size_type __n = _M_bkt_num_key(__key);
      const _Node* __first;
      for (__first = _M_buckets[__n];
           __first && !_M_equals(_M_get_key(__first->_M_val), __key);
           __first = __first->_M_next)
        { }
      return const_iterator(__first, this);
    }

    size_type count(const key_type& __key) const
    {
      const size_type __n = _M_bkt_num_key(__key);
      size_type __result = 0;
      for (const _Node* __cur = _M_buckets[__n]; __cur;
           __cur = __cur->_M_next)
        if (_M_equals(_M_get_key(__cur->_M_val), __key))
          ++__result;
      return __result;
    }

    std::pair<iterator, iterator> equal_range(const key_type& __key);

    std::pair<const_iterator, const_iterator> equal_range(const key_type& __key) const;

    size_type erase(const key_type& __key);
      
    void erase(const iterator& __it);

    void erase(iterator __first, iterator __last);

    void erase(const const_iterator& __it);

    void erase(const_iterator __first, const_iterator __last);

    void resize(size_type __num_elements_hint);

    void clear();

  private:
    size_type _M_next_size(size_type __n) const { return static_cast<size_type>(__stl_next_prime(__n)); }

    void _M_initialize_buckets(size_type __n)
    {
      const size_type __n_buckets = _M_next_size(__n);
      _M_buckets.reserve(__n_buckets);
      _M_buckets.insert(_M_buckets.end(), __n_buckets, (_Node*) 0);
      _M_num_elements = 0;
    }

    size_type _M_bkt_num_key(const key_type& __key) const
    { return _M_bkt_num_key(__key, _M_buckets.size()); }
    
    size_type _M_bkt_num(const value_type& __obj) const
    { return _M_bkt_num_key(_M_get_key(__obj)); }
    
    size_type _M_bkt_num_key(const key_type& __key, std::size_t __n) const
    { return _M_hash(__key) % __n; }
    
    size_type _M_bkt_num(const value_type& __obj, std::size_t __n) const
    { return _M_bkt_num_key(_M_get_key(__obj), __n); }

    _Node* _M_new_node(const value_type& __obj)
    {
      _Node* __n = _M_get_node();
      __n->_M_next = 0;
      try
        {
          this->get_allocator().construct(&__n->_M_val, __obj);
          return __n;
        }
      catch(...)
        {
          _M_put_node(__n);
          throw;
        }
    }

    void _M_delete_node(_Node* __n)
    {
      this->get_allocator().destroy(&__n->_M_val);
      _M_put_node(__n);
    }
      
    void _M_erase_bucket(const size_type __n, _Node* __first, _Node* __last);

    void _M_erase_bucket(const size_type __n, _Node* __last);

    void _M_copy_from(const hashtable& __ht);
  };

  template<class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
  _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>&
  _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>::
  operator++()
  {
    const _Node* __old = _M_cur;
    _M_cur = _M_cur->_M_next;
    if (!_M_cur)
      {
        size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
        while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
          _M_cur = _M_ht->_M_buckets[__bucket];
      }
    return *this;
  }

  template<class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
  inline _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>
  _Hashtable_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>::
  operator++(int)
  {
    iterator __tmp = *this;
    ++*this;
    return __tmp;
  }

  template<class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
  _Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>&
  _Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>::
  operator++()
  {
    const _Node* __old = _M_cur;
    _M_cur = _M_cur->_M_next;
    if (!_M_cur)
      {
        size_type __bucket = _M_ht->_M_bkt_num(__old->_M_val);
        while (!_M_cur && ++__bucket < _M_ht->_M_buckets.size())
          _M_cur = _M_ht->_M_buckets[__bucket];
      }
    return *this;
  }

  template<class _Val, class _Key, class _HF, class _ExK, class _EqK,
           class _All>
  inline _Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>
  _Hashtable_const_iterator<_Val, _Key, _HF, _ExK, _EqK, _All>::
  operator++(int)
  {
    const_iterator __tmp = *this;
    ++*this;
    return __tmp;
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  bool operator==(const hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>& __ht1,
                  const hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>& __ht2)
  {
    typedef typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::_Node _Node;
    
    if (__ht1._M_buckets.size() != __ht2._M_buckets.size())
      return false;
    
    for (std::size_t __n = 0; __n < __ht1._M_buckets.size(); ++__n)
      {
        _Node* __cur1 = __ht1._M_buckets[__n];
        _Node* __cur2 = __ht2._M_buckets[__n];
        // Check same length of lists
        for (; __cur1 && __cur2;
             __cur1 = __cur1->_M_next, __cur2 = __cur2->_M_next)
          { } 
        if (__cur1 || __cur2)
          return false;
        // Now check one's elements are in the other
        for (__cur1 = __ht1._M_buckets[__n] ; __cur1;
             __cur1 = __cur1->_M_next)
          {
            bool _found__cur1 = false;
            for (__cur2 = __ht2._M_buckets[__n];
                 __cur2; __cur2 = __cur2->_M_next)
              {
                if (__cur1->_M_val == __cur2->_M_val)
                  {
                    _found__cur1 = true;
                    break;
                  }
              }
            if (!_found__cur1)
              return false;
          }
      }
    return true;
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  inline bool operator!=(const hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>& __ht1,
                         const hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>& __ht2)
  { return !(__ht1 == __ht2); }

  template<class _Val, class _Key, class _HF, class _Extract, class _EqKey, class _All>
  inline void swap(hashtable<_Val, _Key, _HF, _Extract, _EqKey, _All>& __ht1,
                   hashtable<_Val, _Key, _HF, _Extract, _EqKey, _All>& __ht2)
  { __ht1.swap(__ht2); }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  std::pair<typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::iterator, bool>
  hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::
  insert_unique_noresize(const value_type& __obj)
  {
    const size_type __n = _M_bkt_num(__obj);
    _Node* __first = _M_buckets[__n];
      
    for (_Node* __cur = __first; __cur; __cur = __cur->_M_next)
      if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
        return std::pair<iterator, bool>(iterator(__cur, this), false);
      
    _Node* __tmp = _M_new_node(__obj);
    __tmp->_M_next = __first;
    _M_buckets[__n] = __tmp;
    ++_M_num_elements;
    return std::pair<iterator, bool>(iterator(__tmp, this), true);
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::iterator
  hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::
  insert_equal_noresize(const value_type& __obj)
  {
    const size_type __n = _M_bkt_num(__obj);
    _Node* __first = _M_buckets[__n];
      
    for (_Node* __cur = __first; __cur; __cur = __cur->_M_next)
      if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
        {
          _Node* __tmp = _M_new_node(__obj);
          __tmp->_M_next = __cur->_M_next;
          __cur->_M_next = __tmp;
          ++_M_num_elements;
          return iterator(__tmp, this);
        }

    _Node* __tmp = _M_new_node(__obj);
    __tmp->_M_next = __first;
    _M_buckets[__n] = __tmp;
    ++_M_num_elements;
    return iterator(__tmp, this);
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::reference
  hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::
  find_or_insert(const value_type& __obj)
  {
    resize(_M_num_elements + 1);

    size_type __n = _M_bkt_num(__obj);
    _Node* __first = _M_buckets[__n];
      
    for (_Node* __cur = __first; __cur; __cur = __cur->_M_next)
      if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
        return __cur->_M_val;
      
    _Node* __tmp = _M_new_node(__obj);
    __tmp->_M_next = __first;
    _M_buckets[__n] = __tmp;
    ++_M_num_elements;
    return __tmp->_M_val;
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  std::pair<typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::iterator,
            typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::iterator>
  hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::equal_range(const key_type& __key)
  {
    typedef std::pair<iterator, iterator> _Pii;
    const size_type __n = _M_bkt_num_key(__key);
    
    for (_Node* __first = _M_buckets[__n]; __first;
         __first = __first->_M_next)
      if (_M_equals(_M_get_key(__first->_M_val), __key))
        {
          for (_Node* __cur = __first->_M_next; __cur;
               __cur = __cur->_M_next)
            if (!_M_equals(_M_get_key(__cur->_M_val), __key))
              return _Pii(iterator(__first, this), iterator(__cur, this));
          for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
            if (_M_buckets[__m])
              return _Pii(iterator(__first, this),
                          iterator(_M_buckets[__m], this));
          return _Pii(iterator(__first, this), end());
        }
    return _Pii(end(), end());
  }
  
  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  std::pair<typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::const_iterator,
            typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::const_iterator>
  hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::equal_range(const key_type& __key) const
  {
    typedef std::pair<const_iterator, const_iterator> _Pii;
    const size_type __n = _M_bkt_num_key(__key);
    
    for (const _Node* __first = _M_buckets[__n]; __first;
         __first = __first->_M_next)
      {
        if (_M_equals(_M_get_key(__first->_M_val), __key))
          {
            for (const _Node* __cur = __first->_M_next; __cur;
                 __cur = __cur->_M_next)
              if (!_M_equals(_M_get_key(__cur->_M_val), __key))
                return _Pii(const_iterator(__first, this),
                          const_iterator(__cur, this));
            for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
              if (_M_buckets[__m])
                return _Pii(const_iterator(__first, this),
                            const_iterator(_M_buckets[__m], this));
            return _Pii(const_iterator(__first, this), end());
          }
      }
    return _Pii(end(), end());
  }
  
  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  typename hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::size_type
  hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase(const key_type& __key)
  {
    const size_type __n = _M_bkt_num_key(__key);
    _Node* __first = _M_buckets[__n];
    size_type __erased = 0;
    
    if (__first)
      {
        _Node* __cur = __first;
        _Node* __next = __cur->_M_next;
        while (__next)
          {
            if (_M_equals(_M_get_key(__next->_M_val), __key))
              {
                __cur->_M_next = __next->_M_next;
                _M_delete_node(__next);
                __next = __cur->_M_next;
                ++__erased;
                --_M_num_elements;
              }
            else
              {
                __cur = __next;
                __next = __cur->_M_next;
              }
          }
        if (_M_equals(_M_get_key(__first->_M_val), __key))
          {
            _M_buckets[__n] = __first->_M_next;
            _M_delete_node(__first);
            ++__erased;
            --_M_num_elements;
          }
      }
    return __erased;
  }
  
  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase(const iterator& __it)
  {
    _Node* __p = __it._M_cur;
    if (__p)
      {
        const size_type __n = _M_bkt_num(__p->_M_val);
        _Node* __cur = _M_buckets[__n]; 
        if (__cur == __p)
          {
            _M_buckets[__n] = __cur->_M_next;
            _M_delete_node(__cur);
            --_M_num_elements;
          }
        else
          {
            _Node* __next = __cur->_M_next;
            while (__next)
              {
                if (__next == __p)
                  {
                    __cur->_M_next = __next->_M_next;
                    _M_delete_node(__next);
                    --_M_num_elements;
                    break;
                  }
                else
                  {
                    __cur = __next;
                    __next = __cur->_M_next;
                  }
              }
          }
      }
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase(iterator __first, iterator __last)
  {
    size_type __f_bucket = __first._M_cur ? _M_bkt_num(__first._M_cur->_M_val) : _M_buckets.size();
    
    size_type __l_bucket = __last._M_cur ? _M_bkt_num(__last._M_cur->_M_val) : _M_buckets.size();
    
    if (__first._M_cur == __last._M_cur)
      return;
    else if (__f_bucket == __l_bucket)
      _M_erase_bucket(__f_bucket, __first._M_cur, __last._M_cur);
    else
      {
        _M_erase_bucket(__f_bucket, __first._M_cur, 0);
        for (size_type __n = __f_bucket + 1; __n < __l_bucket; ++__n)
          _M_erase_bucket(__n, 0);
        if (__l_bucket != _M_buckets.size())
          _M_erase_bucket(__l_bucket, __last._M_cur);
      }
  }
  
  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  inline void
  hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::
  erase(const_iterator __first, const_iterator __last)
  {
    erase(iterator(const_cast<_Node*>(__first._M_cur),
                   const_cast<hashtable*>(__first._M_ht)),
          iterator(const_cast<_Node*>(__last._M_cur),
                   const_cast<hashtable*>(__last._M_ht)));
  }
  
  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  inline void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::erase(const const_iterator& __it)
  { erase(iterator(const_cast<_Node*>(__it._M_cur), const_cast<hashtable*>(__it._M_ht))); }
  
  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::resize(size_type __num_elements_hint)
  {
    const size_type __old_n = _M_buckets.size();
    if (__num_elements_hint > __old_n)
      {
        const size_type __n = _M_next_size(__num_elements_hint);
        if (__n > __old_n)
          {
            _Vector_type __tmp(__n, (_Node*)(0), _M_buckets.get_allocator());
            try
              {
                for (size_type __bucket = 0; __bucket < __old_n; ++__bucket)
                  {
                    _Node* __first = _M_buckets[__bucket];
                    while (__first)
                      {
                        size_type __new_bucket = _M_bkt_num(__first->_M_val,__n);
                        _M_buckets[__bucket] = __first->_M_next;
                        __first->_M_next = __tmp[__new_bucket];
                        __tmp[__new_bucket] = __first;
                        __first = _M_buckets[__bucket];
                      }
                  }
                _M_buckets.swap(__tmp);
              }
            catch(...)
              {
                for (size_type __bucket = 0; __bucket < __tmp.size();++__bucket)
                  {
                    while (__tmp[__bucket])
                      {
                        _Node* __next = __tmp[__bucket]->_M_next;
                        _M_delete_node(__tmp[__bucket]);
                        __tmp[__bucket] = __next;
                      }
                  }
                throw;
              }
          }
      }
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::_M_erase_bucket(const size_type __n, _Node* __first, _Node* __last)
  {
    _Node* __cur = _M_buckets[__n];
    if (__cur == __first)
      _M_erase_bucket(__n, __last);
    else
      {
        _Node* __next;
        for (__next = __cur->_M_next;
             __next != __first;
             __cur = __next, __next = __cur->_M_next)
          ;
        while (__next != __last)
          {
            __cur->_M_next = __next->_M_next;
            _M_delete_node(__next);
            __next = __cur->_M_next;
            --_M_num_elements;
          }
      }
  }
  
  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::_M_erase_bucket(const size_type __n, _Node* __last)
  {
    _Node* __cur = _M_buckets[__n];
    while (__cur != __last)
      {
        _Node* __next = __cur->_M_next;
        _M_delete_node(__cur);
        __cur = __next;
        _M_buckets[__n] = __cur;
        --_M_num_elements;
      }
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::clear()
  {
    for (size_type __i = 0; __i < _M_buckets.size(); ++__i)
      {
        _Node* __cur = _M_buckets[__i];
        while (__cur != 0)
          {
            _Node* __next = __cur->_M_next;
            _M_delete_node(__cur);
            __cur = __next;
          }
        _M_buckets[__i] = 0;
      }
    _M_num_elements = 0;
  }

  template<class _Val, class _Key, class _HF, class _Ex, class _Eq, class _All>
  void hashtable<_Val, _Key, _HF, _Ex, _Eq, _All>::_M_copy_from(const hashtable& __ht)
  {
    _M_buckets.clear();
    _M_buckets.reserve(__ht._M_buckets.size());
    _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), (_Node*) 0);
    try
      {
        for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
          const _Node* __cur = __ht._M_buckets[__i];
          if (__cur)
            {
              _Node* __local_copy = _M_new_node(__cur->_M_val);
              _M_buckets[__i] = __local_copy;
              for (_Node* __next = __cur->_M_next;
                   __next;
                   __cur = __next, __next = __cur->_M_next)
                {
                  __local_copy->_M_next = _M_new_node(__next->_M_val);
                  __local_copy = __local_copy->_M_next;
                }
            }
        }
        _M_num_elements = __ht._M_num_elements;
      }
    catch(...)
      {
        clear();
        throw;
      }
  }
}

#endif
