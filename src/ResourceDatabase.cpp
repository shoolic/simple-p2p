//
// Created by przemek on 09.12.2019.
//

#include "ResourceDatabase.h"
#include <utility>
#include <memory>

using std::shared_ptr;
using std::weak_ptr;

namespace simpleP2P {
Resource_Database::Resource_Database(Host localhost) : my_host(std::make_shared<Host>(localhost)) {
  hosts.push_back(my_host);
}

bool Resource_Database::has_file(const Resource &res) {
  std::shared_lock lock(database_mutex);
  auto host = std::find_if(hosts.begin(),
                           hosts.end(),
                           [this](shared_ptr<Host> &it) {
                             return *(it.get()) == *my_host;
                           });
  auto res_i = std::find_if(host->get()->possesed_resources.begin(),
                            host->get()->possesed_resources.end(),
                            [&res](weak_ptr<Resource> &it) {
                              return *(it.lock().get()) == res;
                            });
  return res_i != host->get()->possesed_resources.end();
}

void Resource_Database::add_file(const Resource &res, const Host &host) {
  std::unique_lock lock(database_mutex);
  this->add_file_internal(res, host);
}

bool Resource_Database::remove_file(const Resource &res, const Host &host) {
  std::unique_lock lock(database_mutex);
  auto res_i = std::find_if(resources.begin(),
                            resources.end(),
                            [&res](auto &it) {
                              return *(it.get()) == res;
                            });
  if (res_i != resources.end()) {
    auto host_i = std::find_if(res_i->get()->hosts_in_possession.begin(),
                               res_i->get()->hosts_in_possession.end(),
                               [&host](auto &it) {
                                 return *(it.lock().get()) == host;
                               });
    if (host_i != res_i->get()->hosts_in_possession.end()) {
      host_i->lock()->remove_resource(*res_i);
      res_i->get()->remove_host(host_i->lock());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

shared_ptr<Resource> Resource_Database::who_has_file(std::vector<Uint8> resource_header) {
  return who_has_file(Resource(std::move(resource_header)));
}

shared_ptr<Resource> Resource_Database::who_has_file(const Resource &resource_a) {
  std::shared_lock lock(database_mutex);

  auto resource = std::find_if(resources.begin(),
                               resources.end(),
                               [&resource_a](shared_ptr<Resource> &it) {
                                 return *(it.get()) == resource_a;
                               });
  if (resource != resources.end()) {
    return *resource;
  }
  return std::shared_ptr<Resource>(nullptr);
}

void Resource_Database::add_file(const Resource &res) {
  return add_file(res, *my_host);
}

bool Resource_Database::remove_file(const Resource &res) {
  return remove_file(res, *my_host);
}

std::vector<std::vector<Uint8>> Resource_Database::generate_database_headers() {
  std::vector<std::vector<Uint8>> header;
  std::shared_lock lock(database_mutex);

  auto host = std::find_if(hosts.begin(),
                           hosts.end(),
                           [this](shared_ptr<Host> &it) {
                             return *(it.get()) == *my_host.get();
                           });

  for (auto &it : host->get()->possesed_resources) {
    header.emplace_back(it.lock()->generate_resource_header());

  }

  return header;
}

void Resource_Database::update_host(const Host &host_a) {
  std::unique_lock lock(database_mutex);
  auto host = std::find_if(hosts.begin(),
                           hosts.end(),
                           [&host_a](shared_ptr<Host> &it) {
                             return *(it.get()) == host_a;
                           });
  if (host == hosts.end()) {
    hosts.emplace_back(std::make_shared<Host>(host_a));
    host = hosts.end() - 1;
  }

  shared_ptr<Host> to_update = *host;
  to_update->no_of_missed_updates = 0;

  for (auto &resource: host_a.possesed_resources) {

    if (std::count_if(
        to_update->possesed_resources.begin(),
        to_update->possesed_resources.end(),
        [&resource](auto &it) {
          return *(resource.lock().get()) == *(it.lock().get());
        }) == 0) {
      add_file_internal(*(resource.lock().get()), *(to_update.get()));
    }//else no need for action
  }
}

void Resource_Database::revoke_resource(const Resource &resource_a) {
  std::unique_lock lock(database_mutex);
  auto res = std::find_if(resources.begin(),
                          resources.end(),
                          [resource_a](shared_ptr<Resource> &it) {
                            return *(it.get()) == resource_a;
                          });

  if (res != resources.end()) {
    res->get()->invalidated = true;
    std::for_each(res->get()->hosts_in_possession.begin(),
                  res->get()->hosts_in_possession.end(),
                  [&res](weak_ptr<Host> &it) {
                    it.lock()->remove_resource(*res);
                  });
  }
}

std::shared_ptr<Host> Resource_Database::get_localhost() const {
  return my_host;
}

std::vector<std::shared_ptr<Resource>> Resource_Database::getResources() const {
  std::shared_lock lock(database_mutex);
  return resources;
}

void Resource_Database::check_for_gone_hosts(uint16_t left_margin) {
  std::unique_lock lock(database_mutex);
  for (auto &it : hosts) {
    if (it->no_of_missed_updates >= left_margin) {
      remove_host(it);
    }
  }
}

void Resource_Database::remove_host(const std::shared_ptr<Host> &host) {
  // no lock
  if (host != my_host) {
    for (auto &it: host->possesed_resources) {
      it.lock()->remove_host(host);
    }
    hosts.erase(std::find(hosts.begin(), hosts.end(), host));
  }
}

void Resource_Database::add_file_internal(const Resource &res, const Host &host) {
  // no lock
  auto res_i = std::find_if(resources.begin(),
                            resources.end(),
                            [&res](shared_ptr<Resource> &it) {
                              return *(it.get()) == res;
                            });
  auto host_i = std::find_if(hosts.begin(),
                             hosts.end(),
                             [&host](shared_ptr<Host> &it) {
                               return *(it.get()) == host;
                             });

  //create resource and host if they do not exist
  if (res_i == resources.end()) {
    resources.emplace_back(std::make_shared<Resource>(res));
    res_i = resources.end() - 1;
  }
  if (host_i == hosts.end()) {
    hosts.emplace_back(std::make_shared<Host>(host));
    host_i = hosts.end() - 1;
  }

  //push back pointers
  res_i->get()->hosts_in_possession.emplace_back(*host_i);
  host_i->get()->possesed_resources.push_back(*res_i);
}
}
