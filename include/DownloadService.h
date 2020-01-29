#ifndef SIMPLE_P2P_DOWNLOADSERVICE_H
#define SIMPLE_P2P_DOWNLOADSERVICE_H

#include <boost/asio.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "CompleteResource.h"
#include "DownloadWorker.h"
#include "FileManager.h"
#include "GeneralTypes.h"
#include "LoggingModule.h"
#include "Resource.h"
#include "ResourceDatabase.h"

namespace simpleP2P {

/**
 * @brief Used to download resource.
 *
 */
class DownloadService {
public:
  /**
   * @brief Construct a new Download Service object
   *
   * @param logging_module_c
   * @param io_service_c
   * @param file_manager_c
   * @param resource_database_c
   * @param resource_c
   */
  DownloadService(Logging_Module &logging_module_c,
                  boost::asio::io_service &io_service_c,
                  FileManager &file_manager_c,
                  Resource_Database &resource_database_c,
                  std::shared_ptr<Resource> resource_c);

  ~DownloadService();

  /**
   * @brief Method initiating downloading in the current thread.
   *
   */
  void init();

  /**
   * @brief Method initiating downloading in a new thread.
   *
   * @return std::thread
   */
  std::thread init_thread();

private:
  /**
   * @brief Create a workers object per each not retarded host owning desired
   * resource
   *
   */
  void create_workers();

  /**
   * @brief Spawn worker threads
   *
   */
  void init_workers();

  /**
   * @brief Periodically check if workers exeed timeout.
   * If downloading is completed, download service will asynchronously finished
   * controlling worker threads and will not wait until next awakening.
   *
   */
  void controll_workers();

  /**
   * @brief Method waiting for worker threads to join.
   *
   */
  void join_workers();

  /**
   * @brief Method closing politely all working threads.
   *
   */
  void close_workers();

  /**
   * @brief Method checking workers timeout called periodically by
   * controll_workers method.
   *
   */
  void check_workers_timeout();

  /**
   * @brief Method checking if all workers are unavailable.
   *
   * @return true
   * @return false
   */
  bool all_workers_unavailable();

  /**
   * @brief Method responsible for storing complete resource via File Manager
   * object and adding association between local host and downloaded resource
   * via Resource Database.
   *
   */
  void store_file();

  /**
   * @brief Method responsible for adding association between local host and
   * downloaded resource via Resource Database.
   *
   */
  void add_to_resource_database();

  /**
   * @brief Exception handler.
   *
   * @param e
   */
  void handle_exception(std::exception &e);

  Logging_Module &logging_module;  //!< Logger
  boost::asio::io_service
      &io_service;  //!< Boost IO Service to pass down to download workers
  FileManager &file_manager;  //!< File Manager to store downloaded resource
  Resource_Database
      &resource_database;  //!< Resource Database to add association between
  //!< localhost and resource
  std::shared_ptr<Resource> resource;  //!< Resource to download
  std::shared_ptr<CompleteResource>
      complete_resource;  //!< Complete resource (resource with its data)
  std::vector<std::shared_ptr<DownloadWorker>>
      workers;  //!< Vector of workers, each associtated with one host owning
  //!< resource
  std::vector<std::thread> worker_threads;  //!< Vector of worker threads

  std::condition_variable cv;  //!< Condition variable to wait for timeout
  //!< checking or downloading completion
  std::mutex cv_m;  //!< Mutex for condition variable
};

}  // namespace simpleP2P
#endif  // SIMPLE_P2P_DOWNLOADSERVICE_H
