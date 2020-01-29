//
// Created by przemek on 09.12.2019.
//

#ifndef SIMPLE_P2P_HOST_H
#define SIMPLE_P2P_HOST_H

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include <GeneralTypes.h>
#include <boost/asio.hpp>

namespace simpleP2P {

class Resource; //!< Forward declaration
class UDP_Server;

/**
 * Class contains node information and points to files it possess
 */
class Host {
public:
  /**
   * Constructor
   * @param ip Ip of the Host
   */
  Host(boost::asio::ip::address ip);

  Host(const Host &other) : Host(other.host_ip) {}

  /**
   * Determines if host has resource
   * @param res Resource to be checked
   * @return true if Host has Resource res
   */
  bool has_resource(Resource res);

  /**
   * Operator == checks host_ip for equality
   * @param other other
   * @return true if equal
   */
  bool operator==(const Host &other) const;

  /**
   * Operator != checks host_ip for equality
   * @param other other
   * @return true if not equal
   */
  bool operator!=(const Host &other) const;

  /**
* @brief Method returning boost tcp endpoint (ip address and port) of the
* host.
*
* @return boost::asio::ip::tcp::endpoint
*/
  boost::asio::ip::tcp::endpoint get_endpoint() const;

  /**
   * @brief Method returning true if program considers a given host to be
   * retarded.
   *
   * @return true if a host is retarded
   * @return false otherwise
   */
  bool is_retarded();

  /**
   * @brief Method increasing a host timeout counter.
   * After exeeding timeout limit, a host is considered to be retarded up to
   * some point in time.
   *
   */
  void increase_timeout_counter();

  /**
   * @brief Method returning a time point to which a host is considered to be
   * retarded.
   *
   * @return std::chrono::system_clock::time_point
   */
  std::chrono::system_clock::time_point get_ban_time_point() const;

  const std::vector<std::weak_ptr<Resource>> &get_possesed() const;

private:
  void remove_resource(std::shared_ptr<Resource> res);

  boost::asio::ip::address host_ip;           //!< Ip of the Host

  /*atrribs not checked for equality*/
  Uint16 no_of_missed_updates;
  Int8 timeout_counter;
  bool retarded;
  std::time_t ban_time;

  std::vector<std::weak_ptr<Resource>> possesed_resources; //!< Resources possessed by the Host

  friend class Resource_Database;             //!< friendship to manage Host's Resources timeouts etc
  friend class Udp_Server;
};
}

#endif // SIMPLE_P2P_HOST_H
