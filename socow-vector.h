#pragma once

#include <array>
#include <cstddef>
#include <new>
#include <type_traits>

template <typename T, size_t SMALL_SIZE>
struct socow_vector {
private:
  struct big_storage;

public:
  using iterator = T*;
  using const_iterator = T const*;

  socow_vector();                                     // O(1) nothrow
  socow_vector(socow_vector const&);                  // O(N) strong
  socow_vector& operator=(socow_vector const& other); // O(N) strong

  ~socow_vector(); // O(N) nothrow

  T& operator[](size_t i);             // O(1) nothrow
  T const& operator[](size_t i) const; // O(1) nothrow

  T* data();             // O(1) nothrow
  T const* data() const; // O(1) nothrow
  size_t size() const;   // O(1) nothrow

  T& front();             // O(1) nothrow
  T const& front() const; // O(1) nothrow

  T& back();                // O(1) nothrow
  T const& back() const;    // O(1) nothrow
  void push_back(T const&); // O(1)* strong
  void pop_back();          // O(1) nothrow

  bool empty() const; // O(1) nothrow

  size_t capacity() const; // O(1) nothrow
  void reserve(size_t);    // O(N) strong
  void shrink_to_fit();    // O(N) strong

  void clear(); // O(N) nothrow

  void swap(socow_vector&); // O(1) nothrow

  iterator begin(); // O(1) nothrow
  iterator end();   // O(1) nothrow

  const_iterator begin() const; // O(1) nothrow
  const_iterator end() const;   // O(1) nothrow

  iterator insert(const_iterator pos, T const&); // O(N) strong

  iterator erase(const_iterator pos); // O(N) nothrow(swap)

  iterator erase(const_iterator first,
                 const_iterator last); // O(N) nothrow(swap)

  friend bool operator==(socow_vector& a, socow_vector& b);
  friend bool operator!=(socow_vector& a, socow_vector& b);

private:
  bool is_small;
  size_t size_;
  union {
    T small[SMALL_SIZE];
    big_storage* big;
  };

  struct big_storage {
    size_t capacity;
    size_t ref_count;
    T data[0];

    explicit big_storage(size_t n) : ref_count(1), capacity(n) {}

    void unshare() {
      ref_count--;
    }

    bool is_unique() {
      return ref_count == 1;
    }
    
    bool does_not_exists() {
      return ref_count == 0;
    }
  };

  void swap_stat_dyn(socow_vector&, socow_vector&);

  static big_storage* make_storage(size_t new_capacity) {
    big_storage* ans =
        new (static_cast<big_storage*>(operator new(sizeof(big_storage) +
                                                    new_capacity * sizeof(T))))
            big_storage(new_capacity);
    return ans;
  }

  static void clear_array(T* data_array, size_t len) {
    for (size_t i = len; i > 0; i--) {
      data_array[i - 1].~T();
    }
  }

  static void copy_data_array(const T* src, T* res, size_t src_len) {
    for (size_t i = 0; i < src_len; i++) {
      try {
        new (res + i) T(src[i]);
      }
      catch (...) {
        clear_array(res, i);
        throw;
      }
    }
  }
  
  void make_copy(size_t new_cap) {
    big_storage* tmp = make_storage(new_cap);
    try {
      copy_data_array((is_small ? small : big->data), tmp->data, size_);
    }
    catch (...) {
      operator delete(tmp);
      throw;
    }
    this->~socow_vector();
    big = tmp;
  }

};

template <typename T, size_t SMALL_SIZE>
socow_vector<T, SMALL_SIZE>::socow_vector() : is_small(true), size_(0), big(nullptr) {}

template <typename T, size_t SMALL_SIZE>
socow_vector<T, SMALL_SIZE>::socow_vector(const socow_vector& other) : size_(other.size_), is_small(other.is_small) {
  if (other.is_small) {
    copy_data_array(other.small, small, size_);
  }
  else {
    big = other.big;
    big->ref_count++;
  }
}

template <typename T, size_t SMALL_SIZE>
socow_vector<T, SMALL_SIZE>&
socow_vector<T, SMALL_SIZE>::operator=(socow_vector const& other) {
  if (*this == other) {
    return *this;
  }
  socow_vector(other).swap(*this);
  return *this;
}

template <typename T, size_t SMALL_SIZE>
socow_vector<T, SMALL_SIZE>::~socow_vector() {
  if (is_small) {
    clear_array(small, size_);
    return;
  }
  big->unshare();
  if (big->does_not_exists()) {
    clear_array(big->data, size_);
    operator delete(big);
  }
}


template <typename T, size_t SMALL_SIZE>
T* socow_vector<T, SMALL_SIZE>::data() {
  if (is_small) {
    return small;
  }
  if (!is_small && !big->is_unique()) {
    make_copy(capacity());
  }
  return big->data;
}

template <typename T, size_t SMALL_SIZE>
T const* socow_vector<T, SMALL_SIZE>::data() const {
  return is_small ? small : big->data;
}

template <typename T, size_t SMALL_SIZE>
T& socow_vector<T, SMALL_SIZE>::operator[](size_t i) {
  return data()[i];
}

template <typename T, size_t SMALL_SIZE>
T const& socow_vector<T, SMALL_SIZE>::operator[](size_t i) const {
  return data()[i];
}

template <typename T, size_t SMALL_SIZE>
typename socow_vector<T, SMALL_SIZE>::iterator socow_vector<T, SMALL_SIZE>::begin() {
  return data();
}

template <typename T, size_t SMALL_SIZE>
typename socow_vector<T, SMALL_SIZE>::iterator socow_vector<T, SMALL_SIZE>::end() {
  return data() + size_;
}

template <typename T, size_t SMALL_SIZE>
typename socow_vector<T, SMALL_SIZE>::const_iterator socow_vector<T, SMALL_SIZE>::begin() const {
  return data();
}

template <typename T, size_t SMALL_SIZE>
typename socow_vector<T, SMALL_SIZE>::const_iterator socow_vector<T, SMALL_SIZE>::end() const {
  return data() + size_;
}

template <typename T, size_t SMALL_SIZE>
T& socow_vector<T, SMALL_SIZE>::front() {
  return data()[0];
}

template <typename T, size_t SMALL_SIZE>
T const& socow_vector<T, SMALL_SIZE>::front() const {
  return data()[0];
}

template <typename T, size_t SMALL_SIZE>
T& socow_vector<T, SMALL_SIZE>::back() {
  return data()[size_ - 1];
}

template <typename T, size_t SMALL_SIZE>
T const& socow_vector<T, SMALL_SIZE>::back() const {
  return data()[size_ - 1];
}

template <typename T, size_t SMALL_SIZE>
size_t socow_vector<T, SMALL_SIZE>::size() const {
  return size_;
}

template <typename T, size_t SMALL_SIZE>
bool socow_vector<T, SMALL_SIZE>::empty() const {
  return size_ == 0;
}

template <typename T, size_t SMALL_SIZE>
size_t socow_vector<T, SMALL_SIZE>::capacity() const {
  return is_small ? SMALL_SIZE : big->capacity;
}

template <typename T, size_t SMALL_SIZE>
void socow_vector<T, SMALL_SIZE>::clear() {
  erase(begin(), end());
  return;
}

template <typename T, size_t SMALL_SIZE>
void socow_vector<T, SMALL_SIZE>::push_back(const T& elem) {
  if (size_ == capacity()) {
    size_t new_cap = size_ * 2;
    big_storage* tmp = make_storage(new_cap);
    try {
      copy_data_array(is_small ? small : big->data, tmp->data, size_);
    }
    catch(...) {
      operator delete(tmp);
      throw;
    }
    try {
      new (tmp->data + size_) T(elem);
    }
    catch (...) {
      clear_array(tmp->data, size_);
      operator delete(tmp);
      throw;
    }
    this->~socow_vector();
    big = tmp;
    is_small = false;
  }
  else {
    new (begin() + size_) T(elem);
  }
  size_++;
}

template <typename T, size_t SMALL_SIZE>
void socow_vector<T, SMALL_SIZE>::pop_back() {
  data()[--size_].~T();
}

template <typename T, size_t SMALL_SIZE>
void socow_vector<T, SMALL_SIZE>::swap_stat_dyn(socow_vector& stat, socow_vector& dyn) {
  big_storage* tmp = dyn.big;
  dyn.big = nullptr;
  try {
    copy_data_array(stat.small, dyn.small, stat.size_);
  }
  catch(...) {
    dyn.big = tmp;
    throw;
  }
  clear_array(stat.small, stat.size_);
  stat.big = tmp;
}

template <typename T, size_t SMALL_SIZE>
void socow_vector<T, SMALL_SIZE>::swap(socow_vector& other) {
  if (is_small && other.is_small) {
    for (size_t i = 0; i < std::min<size_t>(size_, other.size_); i++) {
      std::swap(data()[i], other[i]);
    }
    size_t mn = std::min<size_t>(size_, other.size_), mx = std::max<size_t>(size_, other.size_);
    socow_vector& l = (other.size_ < size_ ? other : *this),
                 &b = (other.size_ < size_ ? *this : other);
    for (size_t i = mn; i < mx; ++i) {
      l.push_back(b[i]);
    }
    for (size_t i = mn; i < mx; ++i) {
      b.pop_back();
    }
    return;
  }
  if (!is_small && !other.is_small) {
    std::swap(other.big, big);
  }
  else {
    if (is_small) {
      swap_stat_dyn(*this, other);
    }
    else {
      swap_stat_dyn(other, *this);
    }
  }
  std::swap(size_, other.size_);
  std::swap(is_small, other.is_small);
}

template <typename T, size_t SMALL_SIZE>
void socow_vector<T, SMALL_SIZE>::shrink_to_fit() {
  if (size_ == capacity()) {
    return;
  }
  if (is_small) {
    return;
  }
  if (size_ <= SMALL_SIZE) {
    big_storage* tmp = big;
    big = nullptr;
    try {
      copy_data_array(tmp->data, small, size_);
    }
    catch (...) {
      big = tmp;
      throw;
    }
    tmp->unshare();
    if (tmp->does_not_exists()) {
      clear_array(tmp->data, size_);
      operator delete(tmp);
    }
    is_small = true;
  }
  else {
    make_copy(size_);
  }
}

template <typename T, size_t SMALL_SIZE>
void socow_vector<T, SMALL_SIZE>::reserve(size_t new_cap) {
  if ((!is_small && !big->is_unique()) || new_cap > capacity()) {
    make_copy(std::max<size_t>(new_cap, capacity()));
    is_small = false;
  }
}

template <typename T, size_t SMALL_SIZE>
bool operator==(socow_vector<T, SMALL_SIZE> const& a,
                socow_vector<T, SMALL_SIZE> const& b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (size_t i = 0; i < a.size(); i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

template <typename T, size_t SMALL_SIZE>
bool operator!=(socow_vector<T, SMALL_SIZE> const& a,
                socow_vector<T, SMALL_SIZE> const& b) {
  return a != b;
}

template <typename T, size_t SMALL_SIZE>
typename socow_vector<T, SMALL_SIZE>::iterator
socow_vector<T, SMALL_SIZE>::insert(const_iterator pos, T const& value) {
  size_t p = pos - (is_small ? small : big->data);
  push_back(value);
  T* current = data();
  for (size_t i = size_ - 1; i > p; i--) {
    std::swap(current[i - 1], current[i]);
  }
  return is_small ? small : big->data + p;
}

template <typename T, size_t SMALL_SIZE>
typename socow_vector<T, SMALL_SIZE>::iterator
socow_vector<T, SMALL_SIZE>::erase(const_iterator pos) {
  return pos == ((is_small ? small : big->data) + size_)
           ? end()
           : erase(pos, pos + 1);
}

template <typename T, size_t SMALL_SIZE>
typename socow_vector<T, SMALL_SIZE>::iterator
socow_vector<T, SMALL_SIZE>::erase(const_iterator first, const_iterator last) {
  size_t first_index = first - (is_small ? small : big->data);
  size_t cnt = last - first;
  if (cnt == 0) {
    return begin() + first_index;
  } else {
    T* current = data();
    for (size_t i = first_index; i != size() - cnt; i++) {
      std::swap(current[i], current[i + cnt]);
    }
    for (size_t i = 0; i != cnt; i++) {
      pop_back();
    }
    return is_small ? small : big->data + first_index;
  }
}