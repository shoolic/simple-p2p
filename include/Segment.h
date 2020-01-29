#ifndef SIMPLE_P2P_SEGMENT_H
#define SIMPLE_P2P_SEGMENT_H

#include "GeneralTypes.h"
#include <vector>

namespace simpleP2P {

/**
 * @brief Class representing a segment of a resource.
 * It contains an id of the segment and a pointer to the segment data in
 * complete resource object.
 *
 */
class Segment {
public:
  /**
   * @brief Construct a new Segment object
   *
   * @param id_c segment id
   * @param data_c pointer to the segment data in complete resource object
   */
  Segment(SegmentId id_c, Uint8 *data_c);

  ~Segment();

  /**
   * @brief Get the segment id
   *
   * @return SegmentId
   */
  SegmentId get_id() const;

  /**
   * @brief Get the pointer to the segment data
   *
   * @return Uint8*
   */
  Uint8 *get_data_ptr() const;

  /**
   * @brief
   *
   * @return std::vector<Uint8>
   */
  std::vector<Uint8> serialize_id();

  /**
   * @brief Special method returning Segment object with id set to NO_SEGMENT_ID
   * indicating no segment to download in complete resource object.
   *
   * @return Segment
   */
  static Segment no_segment_left();

  /**
   * @brief Special id number indicating no segment to download in complete
   * resource object.
   *
   */
  static const SegmentId NO_SEGMENT_ID = static_cast<SegmentId>(-1);

private:
  SegmentId id; //!< id of segment
  Uint8 *data;  //!< pointer to segment data
};

} // namespace simpleP2P

#endif // SIMPLE_P2P_SEGMENT_H
