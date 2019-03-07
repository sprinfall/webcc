#ifndef WEBCC_QUEUE_H_
#define WEBCC_QUEUE_H_

// A general message queue.

#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

namespace webcc {

template <typename T>
class Queue {
public:
  Queue() = default;

  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;

  T PopOrWait() {
    std::unique_lock<std::mutex> lock(mutex_);

    // Wait for a message.
    not_empty_cv_.wait(lock, [this] { return !message_list_.empty(); });

    T message = message_list_.front();
    message_list_.pop_front();
    return message;
  }

  T Pop() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (message_list_.empty()) {
      return T();
    }

    T message = message_list_.front();
    message_list_.pop_front();
    return message;
  }

  void Push(const T& message) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      message_list_.push_back(message);
    }
    not_empty_cv_.notify_one();
  }

private:
  std::list<T> message_list_;
  std::mutex mutex_;
  std::condition_variable not_empty_cv_;
};

}  // namespace webcc

#endif  // WEBCC_QUEUE_H_
