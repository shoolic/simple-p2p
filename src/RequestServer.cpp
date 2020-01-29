/*
 * RequestServer.cpp
 * Kamil Zacharczuk
 */

#include "RequestServer.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <memory>   // make_shared
#include <thread>   // thread
#include <utility>  // move
#include "GeneralTypes.h"

using boost::asio::ip::tcp;

namespace simpleP2P {
RequestServer::RequestServer(boost::asio::io_service &_io_service, boost::asio::ip::address my_ip, Uint16 port,
                             FileManager &fm, Logging_Module &lm)
    : io_service(_io_service),
      acceptor(_io_service, tcp::endpoint(my_ip, port)),
      file_manager(fm),
      logging_module(lm) {}

std::thread RequestServer::init() {
  logging_module.add_log_line(
      "RequestServer initialized",
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
  return std::thread([&] { start_accept(); });
}

void RequestServer::init2() {
  logging_module.add_log_line(
      "RequestServer initialized",
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
  start_accept();
}

void RequestServer::start_accept() {
  RequestWorker *new_worker =
      new RequestWorker(io_service, file_manager, logging_module);

  acceptor.async_accept(
      new_worker->socket(),
      boost::bind(&RequestServer::handle_accept, this, new_worker,
                  boost::asio::placeholders::error));
}

void RequestServer::handle_accept(RequestWorker *new_worker,
                                  const boost::system::error_code &error) {
  if (!error) {
    logging_module.add_log_line(
        "RequestServer: successful asynchronous accept",
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    try {
      new_worker->start();
    } catch (std::exception &e) {
      logging_module.add_log_line(
          "RequestServer: exception caught from RequestWorker::start()! "
          "Deleting RequestWorker object",
          std::chrono::system_clock::to_time_t(
              std::chrono::system_clock::now()));

      if (new_worker != nullptr)  // Extra security.
      {
        delete new_worker;
      }
    }
  } else {
    logging_module.add_log_line(
        "RequestServer: ERROR asynchronously accepting",
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    delete new_worker;
  }

  start_accept();
}

/*
void RequestServer::accept_connection()
{
        _acceptor.async_accept(
                // Accept handler.
                [this] (boost::system::error_code err, tcp::socket socket)
                {
                        if (!err)
                        {
                                std::make_shared<RequestWorker>(std::move(socket))->start();
                        }

                        accept_connection(); // Wait for another request.
                });
}*/
}  // namespace simpleP2P
