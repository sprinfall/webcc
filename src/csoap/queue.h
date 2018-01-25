#ifndef CSOAP_QUEUE_H_
#define CSOAP_QUEUE_H_

// A general message queue.

#include <list>

#include "boost/thread/condition_variable.hpp"
#include "boost/thread/locks.hpp"
#include "boost/thread/mutex.hpp"

namespace csoap {

template <typename T>
class Queue {
public:
  Queue(const Queue& rhs) = delete;
  Queue& operator=(const Queue& rhs) = delete;

  Queue() = default;

  T Get() {
    boost::unique_lock<boost::mutex> lock(mutex_);

    // Wait for a message.
    not_empty_cv_.wait(lock, [=] { return !message_list_.empty(); });

    T message = message_list_.front();
    message_list_.pop_front();
    return message;
  }

  void Put(const T& message) {
    {
      boost::lock_guard<boost::mutex> lock(mutex_);
      message_list_.push_back(message);
    }
    not_empty_cv_.notify_one();
  }

private:
  std::list<T> message_list_;
  boost::mutex mutex_;
  boost::condition_variable not_empty_cv_;
};

}  // namespace csoap

#endif  // CSOAP_QUEUE_H_
