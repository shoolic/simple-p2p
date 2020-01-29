//
// Created by przemek on 09.12.2019.
//

#ifndef SIMPLE_P2P_RESOURCE_DATABASE_H
#define SIMPLE_P2P_RESOURCE_DATABASE_H

#include <vector>
#include <string>
#include <shared_mutex>
#include "Host.h"
#include "Resource.h"

namespace simpleP2P {
/**
 * Class holding information about files in network and on localhost
 */
class Resource_Database {
public:
  /**
   * Constructor
   * @param localhost localhost
   */
  Resource_Database(Host localhost);

  /**
   * Check if localhost has certain file
   * @param res Resource to be checked
   * @return true if host already has some resource
   */
  bool has_file(const Resource &res);

  /**
   * Adds connection between file and resource, adn creates them if they do not exist
   * @param res Resource to be added
   * @param host Host which possess Resource res
   */
  void add_file(const Resource &res,
                const Host &host);

  /**
   * Removes connection between file and resource
   * @param res Resource to be removed from host list
   * @param host Host which resource will be removed
   * @return returns false if file did not existed or was not possesed
   */
  bool remove_file(const Resource &res,
                   const Host &host);

  /**
   * Updates the list of resources aviable from host
   * Triggered after receive of full Beacon Packet
   * @param host Host and possesed resources in a struct
   */
  void update_host(const Host &host);

  /**
   * Revokes resource and disconnects it from Hosts in database and database itself
   * Resource will still point to Hosts that possess it
   * @param resource Resource to be revoked
   */
  void revoke_resource(const Resource &resource);

  /**
   * same as add_file(Resource, Host) but host is localhost
   * @param res Resource to be added
   */
  void add_file(const Resource &res);

  /**
    * same as remove_file(Resource, Host) but host is localhost
    * @param res Resource to be removed from localhost list
    * @return returns false if file did not existed or was not possesed
    */
  bool remove_file(const Resource &res);

  /**
  * Returns shared pointer to resource to allow access to information about file owners
  * @param res Resource about which information is gathered
  * @return shared pointer to res
  */
  inline std::shared_ptr<Resource>
  who_has_file(std::vector<Uint8> resource_header);

  /**
   * Returns shared pointer to resource to allow access to information about file owners
   * @param res Resource about which information is gathered
   * @return shared pointer to res
   */
  std::shared_ptr<Resource> who_has_file(const Resource &res);

  /**
   * Generates listing of localhost content in a header
   * @return listing header of localhost resources
   */
  std::vector<std::vector<Uint8>> generate_database_headers();

  /**
   * Get localhost information
   * @return localhost
   */
  std::shared_ptr<Host> get_localhost() const;

  /**
   * Checks if any of the host missed a couple beacon intervals as we suppose they left the network
   * @param left_margin number of left margins needed for deletion of the host
   */
  void check_for_gone_hosts(Uint16 left_margin = LEFT_MARGIN);

  std::vector<std::shared_ptr<Resource>> getResources() const;

private:
  std::shared_ptr<Host> my_host;                     //!< localhost Host struct
  /* all internal operation on this vectors must be made with pointers */
  std::vector<std::shared_ptr<Resource>> resources;  //!< vector of Resources in database
  std::vector<std::shared_ptr<Host>> hosts;          //!< vector of Hosts in database
  std::shared_mutex mutable
      database_mutex;                  //!< rw_lock for database, allows multiple concurrent reads but permits concurrent writes

  inline void remove_host(const std::shared_ptr<Host> &host);

  inline void add_file_internal(const Resource &res, const Host &host);
};
}

#endif //SIMPLE_P2P_RESOURCE_DATABASE_H
