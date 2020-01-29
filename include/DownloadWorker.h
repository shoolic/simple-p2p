#ifndef SIMPLE_DOWNLOADWORKER_H
#define SIMPLE_DOWNLOADWORKER_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>

#include "CompleteResource.h"
#include "GeneralTypes.h"
#include "Segment.h"
#include "Host.h"
#include "LoggingModule.h"
#include "Resource.h"

namespace simpleP2P {
/**
 * @brief Class used to download resource segments from one host.
 *
 */
class DownloadWorker {
public:
  /**
   * @brief Construct a new Download Worker object
   *
   * @param logging_module_c
   * @param io_service_c
   * @param host_c
   * @param complete_resource_c
   */
  DownloadWorker(Logging_Module &logging_module_c,
                 boost::asio::io_service &io_service_c,
                 std::shared_ptr<Host> host_c,
                 std::shared_ptr<CompleteResource> complete_resource_c);

  ~DownloadWorker();

  /**
   * @brief Method initiating connection to host and downloading resource
   * segments in a new thread.
   *
   * @return std::thread
   */
  std::thread init();

  /**
   * @brief Method checking worker timeout and taking appropriate actions if
   * it exeeds:
   * - the host timeout counter is increased, what may cause banning host for
   * certain time,
   * - currently downloaded segment is marked as unbusy and may be
   * downloaded by another worker.
   *
   */
  void check_timeout();

  /**++
   * @brief Synchronised method closing worker politely.
   *
   */
  void close();

  /**
   * @brief Synchronised method checking if worker is closed.
   *
   */
  bool is_closed();
  /**
   * @brief Synchronised method checking if worker is available (not closed and
   * associated host not retarded)
   *
   * @return true
   * @return false
   */
  bool is_unavailable();

private:
  /**
   * @brief Worker loop.
   * Breaks when closed attribute set to true.
   * Suspends the thread until the host is no longer considered to be retarded.
   * Gets segment to download.
   * Downloads segment.
   * Sets segment as completed.
   *
   */
  void worker();

  /**;
   * @brief Connects to host.
   *
   */
  void connect();

  /**
   * @brief Downloads the given segment.
   * Requests the segment from the host.
   * Receives the segment from the host.
   *
   * @param segment
   */
  void download(Segment &segment);

  /**
   * @brief Makes a tcp synchronous write of a serialized segment request.
   *
   * @param segment
   */
  void request_segment(Segment &segment);

  /**
   * @brief Makes a tcp synchronous read to segment data buffer.
   *
   * @param segment
   */
  void receive_segment(Segment &segment);

  /**
   * @brief Serialize the segment request.
   *
   * @param segment
   * @return std::vector<Uint8>
   */
  std::vector<Uint8> serialize_segment_request(Segment &segment);

  /**
   * @brief Log message to logging module when worker exeeds timeout.
   *
   */
  void log_timeout();

  /**
   * @brief Log message to logging module when worker start downloading segment.
   *
   */
  void log_start_downloading();

  /**
   * @brief Log message to logging module while worker finish downloading
   * segment.
   *
   */
  void log_finish_downloading();

  /**
   * @brief Get a prefix to log message identyfing that it comes from Download
   * Worker object associated with the given host, resource and segment.
   *
   */
  std::string get_log_header();

  Logging_Module &logging_module; //!< Logging module
  boost::asio::io_service
      &io_service;            //!< Boost IO Service to TCP communication
  std::shared_ptr<Host> host; //!<  Host to connect and request segments from
  std::shared_ptr<CompleteResource>
      complete_resource;               //!< Complete resource object
  boost::asio::ip::tcp::socket socket; //!< TCP socket
  std::atomic<bool>
      timeouted; //!<  Timeouted flag, reset when one segment downloading
  //!<  completes and set during timeout checking
  std::atomic<bool> closed; //!< Flag for worker loop to break politely
  std::atomic<SegmentId>
      owned_segment_id;       //!< Id of currently downloaded segment
  std::condition_variable cv; //!< Condition variable for waiting until the host
  //!< is no longer considered to be retarded
  std::mutex cv_m;            //!< Mutex for condition variable
  std::mutex timeouted_mutex; //!< Mutex for synchronising timeout actions
};
} // namespace simpleP2P
#endif // SIMPLE_DOWNLOADWORKER_H
