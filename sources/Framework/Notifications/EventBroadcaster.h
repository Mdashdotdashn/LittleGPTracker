#pragma once

#include "System/Console/n_assert.h"

#include <cassert>
#include <vector>
#include <mutex>
#include <algorithm>

template <class T>
class EventBroadcaster
{
public:
  void AddListener(T* pListener)
  {
    std::lock_guard<std::mutex> lock(lock_);
    NAssert(std::find(mListeners.begin(), mListeners.end(), pListener) == mListeners.end());
    mListeners.push_back(pListener);
  }

  void RemoveListener(T* pListener)
  {
    std::lock_guard<std::mutex> lock(lock_);
    auto it = std::find(mListeners.begin(), mListeners.end(), pListener);
    NAssert(it != mListeners.end());
    mListeners.erase(it);
  }

  template <typename... TArgs>
  void NotifyListeners(void (T::*Function)(TArgs...), TArgs... arguments) const
  {
    std::lock_guard<std::mutex> lock(lock_);
    for (T* pListener : mListeners)
    {
      (pListener->*Function)(arguments...);
    }
  }

private:
  std::vector<T*> mListeners;
  mutable std::mutex lock_;
};
