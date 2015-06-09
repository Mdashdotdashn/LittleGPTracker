
#pragma once

#include <cstddef>
#include "System/Console/n_assert.h"

/* class for counted reference semantics
 * - deletes the object to which it refers when the last CountedPtr
 *   that refers to it is destroyed
 */
template <class T>
class CountedPtr 
{
private:
  T* ptr;        // pointer to the value
  long* count;   // shared number of owners
  
public:
  // initialize pointer with existing pointer
  // - requires that the pointer p is a return value of new
  explicit CountedPtr (T* p=0)
  : ptr(p), count(new long(1)) {
  }
  
  // copy pointer (one more owner)
  CountedPtr (const CountedPtr<T>& p) throw()
  : ptr(p.ptr), count(p.count) {
    ++*count;
  }
  
  // destructor (delete value if this was the last owner)
  ~CountedPtr () throw() {
    dispose();
  }
  
  // assignment (unshare old and share new value)
  CountedPtr<T>& operator= (const CountedPtr<T>& p) throw() {
    if (this != &p) {
      dispose();
      ptr = p.ptr;
      count = p.count;
      ++*count;
    }
    return *this;
  }
  
  // assignment (unshare old and share new value)
  CountedPtr<T>& operator= (T* p) throw() {
    *this = CountedPtr<T>(p);
    return *this;
  }

  // access the value to which the pointer refers
  T& operator*() const throw() {
    return *ptr;
  }

  operator T*()
  {
    return ptr;
  }

  operator const T*() const
  {
    return ptr;
  }

  T* operator->() const throw()
  {
    NAssert(!IsNULL());
    return ptr;
  }
  
  bool IsNULL() const
  {
    return (ptr == NULL);
  }
  
  CountedPtr<T> New()
  {
    *this = CountedPtr<T>(new T());
    return *this;
  }

  static CountedPtr<T> SNew()
  {
    return CountedPtr<T>(new T());
  }

private:
  void dispose() {
    if (--*count == 0) {
      delete count;
      delete ptr;
    }
  }
  
};

