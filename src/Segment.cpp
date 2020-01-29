#include "CompleteResource.h"

namespace simpleP2P {
Segment::Segment(SegmentId id_c, Uint8 *data_c) : id(id_c), data(data_c) {}

Segment Segment::no_segment_left() { return Segment{NO_SEGMENT_ID, nullptr}; }

Segment::~Segment() {}

SegmentId Segment::get_id() const { return id; }

Uint8 *Segment::get_data_ptr() const { return data; }
} // namespace simpleP2P
