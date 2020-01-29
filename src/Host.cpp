//
// Created by przemek on 09.12.2019.
//

#include "Host.h"
#include "Resource.h"
#include <iostream>
#include <utility>
#include <vector>

namespace simpleP2P {
Host::Host(boost::asio::ip::address ip)
    : host_ip(std::move(ip)), no_of_missed_updates(0),
      timeout_counter(0), retarded(false) {}

bool Host::has_resource(Resource res) {
  return std::count_if(possesed_resources.begin(), possesed_resources.end(),
                       [&res](std::weak_ptr<Resource> &it) {
                         return res == *(it.lock().get());
                       }) != 0;
}

bool Host::operator==(const Host &other) const {
  return this->host_ip == other.host_ip;
}

bool Host::operator!=(const Host &other) const {
  return this->host_ip != other.host_ip;
}

void Host::remove_resource(std::shared_ptr<Resource> res) {
  possesed_resources.erase(
      std::remove_if(possesed_resources.begin(), possesed_resources.end(),
                     [&res](auto &it) { return it.lock() == res; }),
      possesed_resources.end());
}

const std::vector<std::weak_ptr<Resource>> &Host::get_possesed() const {
  return possesed_resources;
}

bool Host::is_retarded() {
  return ban_time >
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void Host::increase_timeout_counter() {

  timeout_counter++;
  if (timeout_counter >= TIMEOUT_COUNTER_LIMIT) {
    retarded = true;
    using namespace std::chrono_literals;
    ban_time = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now() + BAN_TIME);
  }
}

std::chrono::system_clock::time_point Host::get_ban_time_point() const {
  // return std::chrono::system_clock::from_time_t(ban_time);
  return std::chrono::system_clock::now();
}

boost::asio::ip::tcp::endpoint Host::get_endpoint() const {
  return boost::asio::ip::tcp::endpoint{host_ip, TCP_SERVER_PORT};
}

} // namespace simpleP2P
