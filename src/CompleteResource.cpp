#include "CompleteResource.h"

namespace simpleP2P {

CompleteResource::CompleteResource(std::shared_ptr<Resource> resource_c)
    : resource(resource_c), busy_segments(resource_c->calc_segments_count()),
      completed_segments(resource_c->calc_segments_count()),
      completed_counter(0) {
  data = new Uint8[resource_c->calc_segments_count() * SEGMENT_SIZE];
}

CompleteResource::~CompleteResource() { delete[] data; }

std::shared_ptr<Resource> CompleteResource::get_resource() const {
  return resource;
}

Uint8 *CompleteResource::get_data() {
  return data;
}

Segment CompleteResource::get_segment() {
  if (is_completed()) {
    return Segment::no_segment_left();
  }

  {
    std::unique_lock<std::mutex> lk{complete_resource_mutex};

    for (SegmentId id = 0; id < resource->calc_segments_count(); id++) {
      if (downloadable(id)) {
        busy_segments[id] = true;
        return Segment{id, data + id * SEGMENT_SIZE * sizeof(Uint8)};
      }
    }

    cv.wait(lk);
  } // lock on scope to prevent deadlock while recursing

  return get_segment();
}

void CompleteResource::set_segment(Segment &segment) {
  std::unique_lock<std::mutex> lk{complete_resource_mutex};
  busy_segments[segment.get_id()] = false;
  completed_segments[segment.get_id()] = true;
  completed_counter++;
  // TODO check if notifying all should be there or in is_completed
  // (undesirable side effects)
  if (completed_counter == resource->calc_segments_count()) {
    cv.notify_all();
  }
}

bool CompleteResource::is_completed() {
  // TODO completed_counter is atomic, should there be a lock?
  std::unique_lock<std::mutex> lk{complete_resource_mutex};
  return completed_counter == resource->calc_segments_count();
}

bool CompleteResource::downloadable(SegmentId id) {
  return busy_segments[id] == false && completed_segments[id] == false;
}

void CompleteResource::unset_busy(SegmentId id) {
  std::unique_lock<std::mutex> lk{complete_resource_mutex};
  busy_segments[id] = false;
  cv.notify_one();
}

} // namespace simpleP2P::download
