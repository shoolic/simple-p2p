//
// Created by przemek on 22.12.2019.
//

#include "UdpServer.h"
#include <boost/bind.hpp>
#include <iostream>

using namespace boost::asio;

simpleP2P::Udp_Server::Udp_Server(io_service &io_service,
                                  Resource_Database &database_c, Logging_Module &logger_c,
                                  const boost::asio::ip::address &broadcast_address,
                                  Uint16 broadcast_port)
    : socket_(io_service, ip::udp::endpoint(broadcast_address, broadcast_port)),
      remote_endpoint(), recv_buffer(), database(database_c), logger(logger_c) {
  socket_.set_option(
      ip::udp::socket::reuse_address(true));

  do_receive();
}

void simpleP2P::Udp_Server::handle_receive(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error || error == error::message_size) {

    //FIXME: negate to switch branches while testing on SINGLE host

    if (*database.get_localhost() != Host(remote_endpoint.address())) {
      Uint8 *buf = recv_buffer.data() + 1; // move to header
      if (recv_buffer.front() == FILE_LIST) {
        logger.add_log_line("received (files beacon) : " +
                                std::to_string(bytes_transferred) +
                                " bytes over UDP\n from : " +
                                remote_endpoint.address().to_string(),
                            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        Uint64 iter = be64toh(*(reinterpret_cast<Uint64 *>(buf)));
        buf += 8; //move to first resource
        Host adv_host(remote_endpoint.address());
        std::vector<std::shared_ptr<Resource>> resources; // To keep objects from destruction
        for (Uint64 i = 0; i < iter; ++i) {
          Resource res(std::vector<Uint8>(buf, buf + RESOURCE_HEADER_SIZE));
          buf += RESOURCE_HEADER_SIZE;//move to next header

          resources.push_back(std::make_shared<Resource>(res));
          adv_host.possesed_resources.emplace_back(resources.back());
        }
        database.update_host(adv_host);
        //now resources and adv_host ends scope as they are not longer necessary
      } else if (recv_buffer.front() == REVOKE) {
        logger.add_log_line("received (revocation) : " +
                                std::to_string(bytes_transferred) +
                                " bytes over UDP\n from : " +
                                remote_endpoint.address().to_string(),
                            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

        Resource res(std::vector<Uint8>(buf, buf + 264));

        database.revoke_resource(res); //revoke local Resource information
      }
    }
    do_receive();
  }
}

void simpleP2P::Udp_Server::do_receive() {
  socket_.async_receive_from(
      boost::asio::buffer(recv_buffer), remote_endpoint,
      boost::bind(&Udp_Server::handle_receive, this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

simpleP2P::Udp_Server::~Udp_Server() {
  socket_.close();
}
