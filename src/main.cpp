
#include <DownloadService.h>
#include <FileManager.h>
#include <RequestServerModule.h>

#include <GeneralTypes.h>
#include <LoggingModule.h>
#include <UdpClient.h>
#include <UdpModule.h>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <thread>
#include "CLI.h"

using namespace simpleP2P;
using namespace boost;

int main(int argc, const char *argv[]) {
  std::string host_ip;
  std::string broadcast_ip;

  try {
    program_options::options_description desc{"Options"};
    desc.add_options()("help,h", "Help screen")(
        "my_ip", program_options::value<std::string>(), "Host ip")(
        "broadcast_ip", program_options::value<std::string>(),
        "Broadcast address in host network");

    program_options::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help"))
      std::cout << desc << '\n';
    else {
      if (vm.count("my_ip")) {
        host_ip = vm["my_ip"].as<std::string>();
      } else {
        throw boost::program_options::error("No Host IP");
      }
      if (vm.count("broadcast_ip")) {
        broadcast_ip = vm["broadcast_ip"].as<std::string>();
      } else {
        throw boost::program_options::error("No broadcast IP");
      }
    }
  } catch (const boost::program_options::error &ex) {
    std::cerr << ex.what() << '\n';
  }

  std::thread basic[5];
  /*
   * Create threads for all modules and connect them e.g. by signal-slot
   */

  Host localhost(boost::asio::ip::address::from_string(host_ip));
  Logging_Module logger;
  Resource_Database database(localhost);
  Udp_Module udp(database, logger,
                 boost::asio::ip::address::from_string(broadcast_ip),
                 BROADCAST_PORT,
                 5);  // basic test

  {
    Resource res = Resource("XD", 0);
    database.add_file(res);
  }

  boost::asio::io_service io_service;
  FileManager fm(logger);

  Printer printer(std::cout);
  CLI commandline(database, logger, io_service, fm, *database.get_localhost(),
                  printer, udp);

  RequestServerModule rsm(boost::asio::ip::address::from_string(host_ip),
                          TCP_SERVER_PORT, fm, logger);

  basic[0] = logger.init();
  basic[1] = udp.init();
  basic[2] = printer.init();
  basic[3] = commandline.init();
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(100ms);
  basic[4] = rsm.init();
  printer.print("Init Done\n");

  for (auto &iter : basic) {
    iter.join();
  }
}
