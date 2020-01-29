//
// Created by przemek on 22.12.2019.
//

#include <boost/bind.hpp>
#include "UdpClient.h"

using namespace boost::asio;

simpleP2P::Udp_Client::Udp_Client(io_service &io_service,
                                  Resource_Database &database_c, Logging_Module &logger_c,
                                  const ip::address &broadcast_address,
                                  Uint16 broadcast_port, Uint32 timeout_c)
    : endpoint_(broadcast_address, broadcast_port),
      socket_(io_service, endpoint_.protocol()),
      tx_queue_(),
      timer(io_service, boost::posix_time::seconds(timeout_c)),
      database(database_c), logger(logger_c), timeout(timeout_c) {

  socket_.set_option(ip::udp::socket::reuse_address(true));
  socket_.set_option(socket_base::broadcast(true));

  timer.async_wait(boost::bind(&Udp_Client::fire_beacon,
                               this));
}

simpleP2P::Udp_Client::~Udp_Client() {
  socket_.close();
}

void simpleP2P::Udp_Client::send(const std::vector<Uint8> &packet) {
  bool queue_empty(tx_queue_.empty());
  tx_queue_.emplace_back(packet);
  if (queue_empty)
    transmit();
}

void simpleP2P::Udp_Client::transmit() {
  socket_.async_send_to(
      boost::asio::buffer(&tx_queue_.front()[0], tx_queue_.front().size()),
      endpoint_,
      boost::bind(&Udp_Client::write_callback,
                  boost::weak_ptr<Udp_Client>(shared_from_this()),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void simpleP2P::Udp_Client::write_handler(boost::system::error_code const &error,
                                          size_t bytes_transferred) {
  tx_queue_.pop_front();
  logger.add_log_line("sent : " + std::to_string(bytes_transferred) + " bytes over UDP",
                      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
  if (!error) {
    if (!tx_queue_.empty())
      transmit();
  }
}

void simpleP2P::Udp_Client::write_callback(boost::weak_ptr<Udp_Client> ptr,
                                           boost::system::error_code const &error,
                                           size_t bytes_transferred) {
  boost::shared_ptr<Udp_Client> pointer(ptr.lock());
  if (pointer && (boost::asio::error::operation_aborted != error))
    pointer->write_handler(error, bytes_transferred);
}

void simpleP2P::Udp_Client::revoke_file(simpleP2P::Resource resource) {
  std::vector<Uint8> packet;
  packet.emplace_back(REVOKE);
  auto res = resource.generate_resource_header();
  packet.insert(packet.end(), res.begin(), res.end());
  logger.add_log_line("prepared : revoke over UDP",
                      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
  send(packet);
}

void simpleP2P::Udp_Client::fire_beacon() {
  std::vector<std::vector<Uint8>> files = database.generate_database_headers();

  do {
    std::vector<Uint8> packet;
    Uint16 i;
    if (files.size() >= BEACON_MAX_COUNT) {
      i = BEACON_MAX_COUNT;
    } else {
      i = files.size();
    }

    if (i > 0) {
      packet.resize(sizeof(Uint64) + 1);
      packet[0] = (FILE_LIST);
      Uint64 size_net = htobe64(i);
      memcpy(packet.data() + 1, &size_net, sizeof(size_net));
      //insert resources
      for (; i > 0; --i) {
        auto res = files.back();
        files.pop_back();
        packet.insert(packet.end(), res.begin(), res.end());
      }
      logger.add_log_line("prepared : advertisement over UDP",
                          std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
      send(packet);
    }
  } while (!files.empty());

  database.check_for_gone_hosts();

  boost::posix_time::seconds interval(timeout);
  timer.expires_at(timer.expires_at() + interval);
  timer.async_wait(boost::bind(&Udp_Client::fire_beacon,
                               this));
}
