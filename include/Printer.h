#ifndef SIMPLE_P2P_PRINTER_H
#define SIMPLE_P2P_PRINTER_H

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace simpleP2P {
/**
 * class printing outputs, using queue in order to avoid races
 */
class Printer {
public:
  /**
   * Constructor for the printer
   * @param output_c Output stream for couts
   */
  Printer(std::ostream &output_c = std::cout);

  /**
   * Init methods run worker in thread and returns it
   * @return printing std::thread
   */
  std::thread init();

  /**
   * Synchronised method for printing output
   * @param line
   */
  void print(std::string line);

private:
  /**
   * Main loop of the printer
   */
  void worker();

  std::ostream &output;                         //!< Output stream for couts
  std::queue<std::string> printing_queue;       //!< Queue of dispatched information from running program
  std::mutex queue_mutex;                       //!< Mutex to secure exclusive access to the printer
  std::condition_variable queue_cond;           //!< Cond variable for main loop notification
};

}

#endif //SIMPLE_P2P_PRINTER_H