#include "DownloadService.h"
#include <algorithm>
#include <iostream>
#include <utility>

namespace simpleP2P {
using namespace std::literals;

DownloadService::DownloadService(Logging_Module &logging_module_c,
                                 boost::asio::io_service &io_service_c,
                                 FileManager &file_manager_c,
                                 Resource_Database &resource_database_c,
                                 std::shared_ptr<Resource> resource_c)
    : logging_module(logging_module_c),
      io_service(io_service_c),
      file_manager(file_manager_c),
      resource_database(resource_database_c),
      resource(resource_c) {
  complete_resource = std::make_shared<CompleteResource>(resource_c);
}

DownloadService::~DownloadService() {
  close_workers();
  join_workers();
}

void DownloadService::init() {
  try {
    create_workers();
    init_workers();
    controll_workers();
    store_file();
  } catch (std::exception &e) {
    handle_exception(e);
  }
}

std::thread DownloadService::init_thread() {
  return std::thread([&] { init(); });
}

void DownloadService::create_workers() {
  auto hosts = resource->get_hosts();

  if (hosts.size() == 0) {
    throw std::runtime_error("No host has a resource");
  }

  for (auto host : hosts) {
    if (!host.lock().get()->is_retarded()) {
      workers.push_back(std::make_shared<DownloadWorker>(
          logging_module, io_service, host.lock(), complete_resource));
    }
  }

  if (workers.size() == 0) {
    throw std::runtime_error("Failed to init: no host available");
  }
}

void DownloadService::init_workers() {
  for (auto &worker : workers) {
    worker_threads.push_back(worker->init());
  }
}

void DownloadService::controll_workers() {
  while (true) {
    std::unique_lock<std::mutex> lk{cv_m};
    if (cv.wait_for(lk, TIMEOUT_CHECK_INTERVAL,
                    [this]() { return complete_resource->is_completed(); })) {
      // download_completed
      break;
    } else {
      // timeouted
      if (all_workers_unavailable()) {
        throw std::runtime_error("All workers are unavailable");
      }
      check_workers_timeout();
    }
  }
}

void DownloadService::join_workers() {
  for (auto &worker_thread : worker_threads) {
    worker_thread.join();
  }
}

void DownloadService::check_workers_timeout() {
  for (auto &worker : workers) {
    worker->check_timeout();
  }
}

void DownloadService::close_workers() {
  for (auto &worker : workers) {
    worker->close();
  }
}

bool DownloadService::all_workers_unavailable() {
  return std::all_of(workers.begin(), workers.end(),
                     [](auto worker) { return worker->is_unavailable(); });
}

void DownloadService::handle_exception(std::exception &e) {
  std::string error_message =
      "Failed to download resource: " + resource->getName() +
          "\n detailed error: " + e.what() + "\n";
  using namespace std::chrono;
  logging_module.add_log_line(error_message,
                              system_clock::to_time_t(system_clock::now()));
}

void DownloadService::store_file() {
  resource_database.add_file(*resource);
  file_manager.store_resource(complete_resource);
}

}  // namespace simpleP2P
