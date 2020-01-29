/*
 * RequestServer.h
 * Kamil Zacharczuk
 */
#ifndef SIMPLE_P2P_REQUEST_SERVER_H
#define SIMPLE_P2P_REQUEST_SERVER_H

#include <boost/asio.hpp>
#include <thread>
#include "GeneralTypes.h"
#include "RequestWorker.h"
using boost::asio::ip::tcp;

namespace simpleP2P {
class FileManager;
/**
 * Asynchronous TCP server. It accepts connections asynchronously and for each
 * of them creates a worker object to handle it.
 */
class RequestServer {
public:
  /**
   * Constructor allows setting the parameters for the connnetion acceptor.
   *
   * @param io_service boost::asio::io_service for the acceptor.
   * @param port Port for the acceptor to listen on.
   */
  RequestServer(boost::asio::io_service &io_service, boost::asio::ip::address my_ip, Uint16 port,
                FileManager &fm, Logging_Module &lm);

  /**
   * Turns on the listening and accepting connections and returns the thread in
   * which it works.
   */
  std::thread init();
  void init2();

private:
  boost::asio::io_service &
      io_service;  //!< io_service in which the server runs.
  tcp::acceptor
      acceptor;  //!< The acceptor listening and accepting connections.
  FileManager &file_manager;  //!< FileManager for accessing requested files.
  Logging_Module &logging_module;  //!< Logging_Module for logging events.

  /**
   * \brief Asynchronous connection accepting function.
   *
   * When a connection is requested, it creates a worker object to handle it and
   * accepts it, calling the handling function.
   */
  void start_accept();

  /**
   * Starts the worker so it handles the connection if it's accepted
   * successfully, and calls listening for next connections.
   */
  void handle_accept(RequestWorker *new_worker,
                     const boost::system::error_code &error);
};
}  // namespace simpleP2P

#endif  // SIMPLE_P2P_REQUEST_SERVER_H
