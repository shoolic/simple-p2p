/*
 * RequestServerModule.h
 * Kamil Zacharczuk
 */
#ifndef SIMPLE_P2P_REQUEST_SERVER_MODULE_H
#define SIMPLE_P2P_REQUEST_SERVER_MODULE_H

#include <thread>
#include "GeneralTypes.h"
#include "boost/asio.hpp"
#include "LoggingModule.h"

namespace simpleP2P {
class FileManager;
/**
 * Module of the TCP server receiving file requests and sending the requested
 * files' segments.
 */
class RequestServerModule {
public:
  /**
   * Constructor, allows setting the port for the server.
   */
  RequestServerModule(boost::asio::ip::address my_ip_c, Uint16 port_,
                      FileManager &fm, Logging_Module &lm);

  /**
   * \brief Returns the thread object for the module.
   *
   * Starts the server and returns the thread in which the server works.
   */
  std::thread init();

private:
  boost::asio::ip::address my_ip;  //!< IP for the server.
  Uint16 port;                     //!< Port for the server.
  FileManager &file_manager;  //!< FileManager for accessing requested files.
  Logging_Module &logging_module;  //!< Logging_Module for logging events.
};
}  // namespace simpleP2P

#endif  // SIMPLE_P2P_REQUEST_SERVER_MODULE_H
