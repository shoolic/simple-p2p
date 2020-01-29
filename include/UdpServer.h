//
// Created by przemek on 22.12.2019.
//

#ifndef SIMPLE_P2P_UDP_SERVER_H
#define SIMPLE_P2P_UDP_SERVER_H

#include <boost/asio/ip/udp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include "GeneralTypes.h"
#include "ResourceDatabase.h"
#include "LoggingModule.h"

namespace simpleP2P {
/**
 * class UDP Server to handle all incoming packets
 */
class Udp_Server : public boost::enable_shared_from_this<Udp_Server> {
public:
  /**
   * Constructor of UDP Server
   * @param io_service asio Io Service
   * @param database Database
   * @param logger Logger
   * @param broadcast_address address on which Server will listen
   * @param broadcast_port port on which Server will listen
   */
  Udp_Server(boost::asio::io_service &io_service,
             Resource_Database &database, Logging_Module &logger,
             const boost::asio::ip::address &broadcast_address,
             Uint16 broadcast_port);

  /**
   * Destructor closes socket
   */
  ~Udp_Server();

private:
  /**
   * Function listening for incoming packets
   */
  void do_receive();

  /**
   * Function called to handle received data
   * @param error error code
   * @param bytes_transferred number of received bytes
   */
  void handle_receive(const boost::system::error_code &error,
                      std::size_t bytes_transferred);

  boost::asio::ip::udp::socket socket_;                //!< Socket on which operates Server
  boost::asio::ip::udp::endpoint remote_endpoint;      //!< Endpoint from data has came
  boost::array<Uint8, UDP_SERV_BUFFER_SIZE> recv_buffer;//!< Buffer for received data
  Resource_Database &database;                         //!< Connection to ResourceDatabase
  Logging_Module &logger;                              //!< Connection to LoggingModule
};
}

#endif //SIMPLE_P2P_UDP_SERVER_H
