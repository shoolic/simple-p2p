#include "DownloadWorker.h"
#include <sstream>

namespace simpleP2P {

DownloadWorker::DownloadWorker(
    Logging_Module &logging_module_c, boost::asio::io_service &io_service_c,
    std::shared_ptr<Host> host_c,
    std::shared_ptr<CompleteResource> complete_resource_c)
    : logging_module(logging_module_c),
      io_service(io_service_c),
      host(host_c),
      complete_resource(complete_resource_c),
      socket(io_service_c),
      timeouted(false),
      closed(false),
      owned_segment_id(Segment::NO_SEGMENT_ID) {}

DownloadWorker::~DownloadWorker() {
  if (socket.is_open()) {
    socket.close();
  }
}

std::thread DownloadWorker::init() {
  return std::thread([&] {
    try {
      connect();
      logging_module.add_log_line("new DownloadWorker initiated",
                                  std::chrono::system_clock::to_time_t(
                                      std::chrono::system_clock::now()));
      worker();
      logging_module.add_log_line("DownloadWorker finished",
                                  std::chrono::system_clock::to_time_t(
                                      std::chrono::system_clock::now()));
    } catch (std::exception &e) {
      closed = true;
      std::stringstream error_message;
      error_message << "Download worker terminated, host: "
                    << host->get_endpoint() << std::endl
                    << " detailed error: " << e.what() << std::endl;

      using namespace std::chrono;
      logging_module.add_log_line(error_message.str(),
                                  system_clock::to_time_t(system_clock::now()));
    }
  });
}

void DownloadWorker::worker() {
  while (!closed) {
    if (host->is_retarded()) {
      std::unique_lock<std::mutex> lk{cv_m};
      cv.wait_until(lk, host->get_ban_time_point(),
                    [this]() { return closed == true; });
    }

    Segment segment = complete_resource->get_segment();

    if (segment.get_id() == Segment::NO_SEGMENT_ID) {
      break;
    }

    owned_segment_id = segment.get_id();
    log_start_downloading();
    download(segment);
    log_finish_downloading();
  }
}

void DownloadWorker::close() {
  std::unique_lock<std::mutex> lk(cv_m);
  closed = true;
  cv.notify_one();
  socket.cancel();
  boost::system::error_code error_code;
  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error_code);
}

void DownloadWorker::connect() {
  try {
    socket.connect(host->get_endpoint());
  } catch (std::exception &e) {
    std::stringstream error_message;
    error_message << "Failed to connect to host: " << host->get_endpoint()
                  << std::endl
                  << " detailed error: " << e.what() << std::endl;

    using namespace std::chrono;
    logging_module.add_log_line(error_message.str(),
                                system_clock::to_time_t(system_clock::now()));
    throw std::runtime_error("Connection failure");
  }
}

void DownloadWorker::download(Segment &segment) {
  try {
    request_segment(segment);
    receive_segment(segment);
    timeouted = false;
    complete_resource->set_segment(segment);
  } catch (std::exception &e) {
    std::stringstream error_message;
    error_message << "Failed to download segment: " + segment.get_id()
                  << std::endl
                  << " of resource: "
                  << complete_resource->get_resource()->getName() << std::endl
                  << " from host " << host->get_endpoint() << std::endl
                  << " detailed error: " << e.what() << std::endl;

    using namespace std::chrono;
    logging_module.add_log_line(error_message.str(),
                                system_clock::to_time_t(system_clock::now()));

    complete_resource->unset_busy(owned_segment_id);
    throw e;
  }
}

void DownloadWorker::request_segment(Segment &segment) {
  boost::system::error_code error;
  std::vector<Uint8> data = serialize_segment_request(segment);

  boost::asio::write(socket, boost::asio::buffer(data), error);
  if (error) {
    throw boost::system::system_error(error);
  }
}

void DownloadWorker::receive_segment(Segment &segment) {
  boost::system::error_code error;
  boost::asio::read(
      socket, boost::asio::buffer(segment.get_data_ptr(), SEGMENT_SIZE), error);
  if (error) {
    throw boost::system::system_error(error);
  }
}

std::vector<Uint8> DownloadWorker::serialize_segment_request(Segment &segment) {
  std::vector<Uint8> data;
  data.push_back(REQ_SEGMENT);

  std::vector<Uint8> resource_header =
      complete_resource->get_resource()->generate_resource_header();

  std::copy(resource_header.begin(), resource_header.end(),
            std::back_inserter(data));

  auto id = htons(segment.get_id());
  const Uint8 *byte_id_begin = reinterpret_cast<const Uint8 *>(&id);
  std::copy(byte_id_begin, byte_id_begin + sizeof(SegmentId),
            std::back_inserter(data));

  return data;
}

void DownloadWorker::check_timeout() {
  std::unique_lock<std::mutex> lk{timeouted_mutex};
  if (timeouted) {
    host->increase_timeout_counter();
    complete_resource->unset_busy(owned_segment_id);
  }

  timeouted = true;
}

void DownloadWorker::log_timeout() {
  logging_module.add_log_line(
      get_log_header() + "timeout",
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

void DownloadWorker::log_start_downloading() {
  logging_module.add_log_line(
      get_log_header() + "segment downloading started",
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

void DownloadWorker::log_finish_downloading() {
  logging_module.add_log_line(
      get_log_header() + "segment downloading finished",
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

std::string DownloadWorker::get_log_header() {
  std::stringstream message;
  message << "Download worker"
          << "; Resource: " << complete_resource->get_resource()->getName()
          << "; Host " << host->get_endpoint()
          << "; Segment: " << owned_segment_id << "; Message: ";
  return message.str();
}

bool DownloadWorker::is_closed() { return closed; }
bool DownloadWorker::is_unavailable() { return closed || host->is_retarded(); }

}  // namespace simpleP2P
