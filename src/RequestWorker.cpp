/*
 * RequestWorker.cpp
 * Kamil Zacharczuk
 */
#include <cstddef> // size_t
#include <utility> // move
#include <algorithm> // find	
#include <string>
#include <sstream> // stringstream
#include <cmath> // modf
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "RequestWorker.h"
#include "GeneralTypes.h"

using boost::asio::ip::tcp;

namespace simpleP2P {
RequestWorker::RequestWorker(boost::asio::io_service &io_service, FileManager &fm, Logging_Module &lm)
    : _socket(io_service), file_manager(fm), logging_module(lm) {}

RequestWorker::~RequestWorker() {
  if (send_data != nullptr) {
    delete send_data;
  }
}

void RequestWorker::start() {
  send_data = static_cast<Uint8 *>(malloc(SEGMENT_SIZE));

  if (send_data == nullptr) {
    logging_module.add_log_line(
        "RequestWorker::start(): allocating memory for a segment to be sent FAILED! (malloc returned nullptr) Throwing 																	   exception...",
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

    throw std::exception();
  }

  logging_module.add_log_line("RequestWorker successfully started",
                              std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

  _socket.async_read_some(boost::asio::buffer(recv_data, MAX_RECV_DATA_LENGHT),
                          boost::bind(&RequestWorker::handle_read, this,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
}

tcp::socket &RequestWorker::socket() {
  return _socket;
}

void RequestWorker::handle_read(const boost::system::error_code &error,
                                std::size_t bytes_transferred) {
  (void) bytes_transferred;
  if (!error) {
    // Get the command
    Uint8 command = recv_data[0];

    // Get the file_name from the recv_data buffer (it's null-terminated)
    auto end = std::find(recv_data + 1, recv_data + 1 + FILE_NAME_LENGHT, '\0');
    std::string file_name((recv_data + 1), end);

    if (command == REQ_SEGMENT) {
      Uint64 file_size;
      std::memcpy(&file_size, recv_data + 1 + FILE_NAME_LENGHT, FILE_SIZE_LENGHT);
      file_size = be64toh(file_size);

      Uint16 segment;
      std::memcpy(&segment, recv_data + 1 + FILE_NAME_LENGHT + FILE_SIZE_LENGHT, sizeof(Uint16));
      // Uint16 is always 16-bit, not platform dependent.
      segment = ntohs(segment);

      std::string logmsg =
          "RequestWorker: request for segment " + std::to_string(segment) + " of file: '" + file_name + "' received";
      logging_module.add_log_line(logmsg, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

      Uint32 requested_segment_size;
      double nsegments = static_cast<double>(file_size) / SEGMENT_SIZE;

      // Check if the requested segment is the last segment of the file (and its size != SEGMENT_SIZE).
      // If this is the case, calculate its size.
      if (segment == static_cast<Uint16>(nsegments)) {
        requested_segment_size = file_size - static_cast<Uint16>(nsegments) * SEGMENT_SIZE;
      } else {
        requested_segment_size = SEGMENT_SIZE;
      }

      /* Explicit block in which 'send_data' is declared */
      file_manager.read_lock(file_name);

      if (!file_manager.get_segment(file_name, segment, send_data, requested_segment_size)) {
        logging_module.add_log_line(
            "RequestWorker: getting the requested segment from file manager FAILED. Deleting RequestWorker object",
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        delete this;
      } else {
        logmsg = "RequestWorker: sending segment " + std::to_string(segment) + " of file: '" + file_name + "'...";
        logging_module.add_log_line(logmsg, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        // Send the segment.
        boost::asio::async_write(_socket,
                                 boost::asio::buffer(send_data, std::size_t(SEGMENT_SIZE)),
                                 boost::bind(&RequestWorker::handle_write, this, boost::asio::placeholders::error));
      }

    } else if (command == QUIT_CONN) {
      logging_module.add_log_line("RequestWorker: client requested a connection quit. Deleting RequestWorker object",
                                  std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
      file_manager.read_unlock(file_name);

      delete this;
    }
  } else {
    if ((boost::asio::error::eof == error) ||
        (boost::asio::error::connection_reset == error)) {
      logging_module.add_log_line("RequestWorker: Disconnected. Deleting RequestWorker object",
                                  std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    } else {
      logging_module.add_log_line("RequestWorker: ERROR reading data from the client! Deleting RequestWorker object",
                                  std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    }
    delete this;
  }
}

void RequestWorker::handle_write(const boost::system::error_code &error) {
  if (!error) {
    logging_module.add_log_line("RequestWorker: segment successfully sent",
                                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

    // Get back to listening for further requests.
    _socket.async_read_some(boost::asio::buffer(recv_data, MAX_RECV_DATA_LENGHT),
                            boost::bind(&RequestWorker::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  } else {
    logging_module.add_log_line("RequestWorker: ERROR sending a segment! Deleting RequestWorker object",
                                std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

    // Kill yourself, you can't even send a mere kilobyte.
    delete this;
  }
}
}
