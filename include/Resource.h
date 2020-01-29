//
// Created by przemek on 09.12.2019.
//

#ifndef SIMPLE_P2P_RESOURCE_H
#define SIMPLE_P2P_RESOURCE_H

#include <vector>
#include <string>
#include <GeneralTypes.h>
#include "tbb/concurrent_vector.h"

#define SEGMENT_SIZE 1024 // 1kb

namespace simpleP2P {

class Host; //!< Forward declaration

/**
 * Class contains file information and points to nodes with file possesion
 */
class Resource {
public:
  /**
   * Constructor
   * @param name filename
   * @param size filesize
   * @param path filepath, default is "./"
   */
  Resource(std::string name, Uint64 size, std::string path = "./");

  /**
   * Constructor makes resource from header
   * @param resource_header Resource header
   */
  Resource(std::vector<Uint8> resource_header);

  Resource(const Resource &other) : Resource(other.name, other.size, other.path) {}

  /**
   * Generates Resource header
   * @return Resource header
   */
  std::vector<Uint8> generate_resource_header();

  /**
   * Determines if resource is possesed by Host
   * @param host Host
   * @return true if resource is possessed by host
   */
  bool has_host(Host host);

  /**
   * Calculates and returns segment count
   * @return segment count
   */
  Uint16 calc_segments_count() const {
      if (size % SEGMENT_SIZE)
          return 1 + (size / SEGMENT_SIZE);
      else
          return size / SEGMENT_SIZE;
  }

  /**
   * Function used to set invalidated flag.
   * To allow references on resource outside database to gather information about revoke
   */
  inline void set_revoked() {
      invalidated = true;
  }

  bool isInvalidated();

  /**
   * Getter for file size
   * @return file size
   */
  Uint64 getSize() const;

  /**
   * Getter for file name
   * @return file name
   */
  const std::string &getName() const;

  /**
   * Getter for file path
   * @return file path
   */
  const std::string &getPath() const;

  /**
   * Operator == checks file size and name for equality
   * @param other other
   * @return true if equal
   */
  bool operator==(const Resource &other) const;

  /**
   * Operator != checks file size and name for equality
   * @param other other
   * @return true if not equal
   */
  bool operator!=(const Resource &other) const;

  // const tbb::concurrent_vector<std::weak_ptr<Host>>
  const std::vector<std::weak_ptr<Host>> get_hosts() const;

private:
  void remove_host(std::shared_ptr<Host> host);

  Uint64 size;                            //!< file size
  std::string name;                       //!< file name
  /*atrribs not checked for equality*/
  bool invalidated;                       //!< indicates that resource has been revoked
  std::string path;                       //!< file path
  // tbb::concurrent_vector<std::weak_ptr<Host>>
  std::vector<std::weak_ptr<Host>> hosts_in_possession;  //!< Host in possession of the Resource

  friend class Resource_Database;         //!< friendship to manage Resource Hosts, path etc
};
}

#endif //SIMPLE_P2P_RESOURCE_H
