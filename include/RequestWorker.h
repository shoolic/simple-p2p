/*
 * RequestWorker.h
 * Kamil Zacharczuk
 */
#ifndef SIMPLE_P2P_REQUEST_WORKER_H
#define SIMPLE_P2P_REQUEST_WORKER_H

#include <memory>    // enable_shared_from_this
#include <boost/asio.hpp>

#include "FileManager.h"
#include "LoggingModule.h"
#include "GeneralTypes.h"

using boost::asio::ip::tcp;

namespace simpleP2P {
/**
 * \brief TCP connection handler, created by the TCP server.
 *
 * Receives the file request, buffers requested segments and sends them to the client.
 */
class RequestWorker {
public:
  /**
   * Constructor allows setting the socket on which the connection is established
   *
   * @param io_service io_service in which the server runs
   * @param fm FileManager for accessing requested files
   * @param lm Logging_Module for logging events
   */
  RequestWorker(boost::asio::io_service &io_service, FileManager &fm, Logging_Module &lm);

  ~RequestWorker();

  /**
   * Start handling the request.
   */
  void start();

  /**
   * Get socket.
   */
  tcp::socket &socket();

private:
  static const Uint16 MAX_RECV_DATA_LENGHT = 300;  //!< Maximum data that can be read via TCP.

  tcp::socket _socket;                             //!< Socket on which the connection is established.
  FileManager &file_manager;                       //!< FileManager for accessing requested files.
  Logging_Module &logging_module;                  //!< Logging_Module for logging events.

  Uint8 recv_data[MAX_RECV_DATA_LENGHT];          //!< Buffer for data received via TCP.
  Uint8 *send_data;                                //!< Buffer for data to be sent via TCP.

  /**
   * Handler called when reading data via TCP.
   *
   * @param error The reading error code
   * @param bytes_transferred Number of bytes read
   */
  void handle_read(const boost::system::error_code &error, std::size_t bytes_transferred);
  /**
   * Handler called when sending data via TCP.
   *
   * @param error The writing error code
   */
  void handle_write(const boost::system::error_code &error);
};
}

#endif // SIMPLE_P2P_REQUEST_WORKER_H
