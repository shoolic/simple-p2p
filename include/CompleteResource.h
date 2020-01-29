#ifndef SIMPLE_P2P_COMPLETERESOURCE_H
#define SIMPLE_P2P_COMPLETERESOURCE_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

#include <boost/dynamic_bitset.hpp>

#include "GeneralTypes.h"
#include "Segment.h"
#include "Resource.h"

namespace simpleP2P {
/**
 * @brief Class representing a complete resource (resource and full its data)
 *
 */
class CompleteResource {
public:
  /**
   * @brief Construct a new Complete Resource object based on base Resource
   * object
   *
   * @param resource_c base resource object
   */
  CompleteResource(std::shared_ptr<Resource> resource_c);

  ~CompleteResource();

  /**
  * @brief Get the contents of the file.
  *
  * @return Uint8*
  */
  Uint8 *get_data();

  /**
   * @brief Get the underlaying resource object
   *
   * @return std::shared_ptr<Resource>
   */
  std::shared_ptr<Resource> get_resource() const;

  /**
   * @brief Synchronised method returning Segment object representing the first
   * unbusy and incomplete segment.
   * If resource downlaoding has been completed, special Segment object with
   * id set to NO_SEGMENT_ID is returned.
   * If there is no appropriate segment found and downloading is not completed,
   * method suspends calling worker thread pending some segment release.
   *
   * @return Segment
   */
  Segment get_segment();

  /**
   * @brief Synchronised method marking the given segment as completed.
   * If all segments have been downloaded, method notifies all waiting worker
   * threads so that they can join.
   *
   * @param segment
   */
  void set_segment(Segment &segment);

  /**
   * @brief  Synchronised method returning true if all segments have been
      downloaded, false otherwise.

   * @return true if downloading is completed
   * @return false otherwise
   */
  bool is_completed();

  /**
   * @brief Synchronised method unmarking busy segment and notifying one of
   * waithing worker threads.
   *
   * @param id
   */
  void unset_busy(SegmentId id);

private:
  /**
   * @brief Checks if the segment with the given is downloadable.: it is not
   * busy and not completed
   *
   * @param id
   * @return true if  it is not busy and not completed
   * @return false otherwise
   */
  bool downloadable(SegmentId id);

  std::shared_ptr<Resource> resource; //!< Resource
  boost::dynamic_bitset<>
      busy_segments; //!< Bitset of busy segments - 1 means assigned to download
  boost::dynamic_bitset<>
      completed_segments; //!< Bitset of completed segments - 1 means segment
  //!< has been downloaded
  SegmentId completed_counter; //!< Counter of downloaded segments
  Uint8 *data;                 //!< Pointer to reseource data
  std::condition_variable cv; //!< Condition variable for worker threads to wait
  //!< for segment to download
  std::mutex complete_resource_mutex; //!< Synchronization object
};

} // namespace simpleP2P
#endif // SIMPLE_P2P_COMPLETERESOURCE_H
