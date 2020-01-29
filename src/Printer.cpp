#include "Printer.h"

namespace simpleP2P {
Printer::Printer(std::ostream &output_c) : output(output_c) {}

std::thread Printer::init() {
  return std::thread([&] { worker(); });
}

void Printer::worker() {
  std::unique_lock<std::mutex> uniqueLock(queue_mutex);
  for (;;) {
    queue_cond.wait(uniqueLock);
    while (printing_queue.size()) {
      output << printing_queue.front();
      printing_queue.pop();
    }
  }
}

void Printer::print(std::string line) {
  std::lock_guard<std::mutex> uniqueLock(queue_mutex);
  printing_queue.push(line);
  queue_cond.notify_one();
}
}