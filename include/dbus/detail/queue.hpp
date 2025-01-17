// Copyright (c) Benjamin Kietzman (github.com/bkietz)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef DBUS_QUEUE_HPP
#define DBUS_QUEUE_HPP

#include <boost/asio.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <deque>

#include <dbus/functional.hpp>

namespace dbus {
namespace detail {

template <typename Message> class queue {
public:
  typedef ::boost::asio::detail::mutex mutex_type;
  typedef Message message_type;
  typedef function<void(boost::system::error_code, Message)> handler_type;

private:
  boost::asio::io_service &io;
  mutex_type mutex;
  std::deque<message_type> messages;
  std::deque<handler_type> handlers;

public:
  queue(boost::asio::io_service &io_service) : io(io_service) {}

private:
  class closure {
    handler_type handler_;
    message_type message_;
    boost::system::error_code error_;

  public:
    void operator()() { handler_(error_, message_); }
    closure(BOOST_ASIO_MOVE_ARG(handler_type) h, Message m,
            boost::system::error_code e = boost::system::error_code())
        : handler_(h), message_(m), error_(e) {}
  };

public:
  void push(message_type m) {
    mutex_type::scoped_lock lock(mutex);
    if (handlers.empty())
      messages.push_back(m);
    else {
      handler_type h = handlers.front();
      handlers.pop_front();

      lock.unlock();

      io.post(closure(BOOST_ASIO_MOVE_CAST(handler_type)(h), m));
    }
  }

  template <typename MessageHandler>
  inline BOOST_ASIO_INITFN_RESULT_TYPE(MessageHandler,
                                       void(boost::system::error_code,
                                            message_type))
      async_pop(BOOST_ASIO_MOVE_ARG(MessageHandler) h) {

#if BOOST_VERSION >= 106700
    typedef ::boost::asio::async_completion<
        MessageHandler, void(boost::system::error_code, message_type)>
        init_type;
#else
    typedef ::boost::asio::detail::async_result_init<
        MessageHandler, void(boost::system::error_code, message_type)>
        init_type;
#endif

    mutex_type::scoped_lock lock(mutex);
    if (messages.empty()) {
      init_type init(h);

#if BOOST_VERSION >= 106700
      handlers.push_back(init.completion_handler);
#else
      handlers.push_back(init.handler);
#endif

      lock.unlock();

      return init.result.get();

    } else {
      message_type m = messages.front();
      messages.pop_front();

      lock.unlock();

      init_type init(h);

      io.post(closure(
#if BOOST_VERSION >= 106700
          BOOST_ASIO_MOVE_CAST(handler_type)(init.completion_handler), m));
#else
          BOOST_ASIO_MOVE_CAST(handler_type)(init.handler), m));
#endif

      return init.result.get();
    }
  }
};

} // namespace detail
} // namespace dbus

#endif // DBUS_QUEUE_HPP
