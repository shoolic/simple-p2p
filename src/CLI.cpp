#include "CLI.h"
#include "DownloadService.h"
#include "CLICommand.h"
#include "ResourceDatabase.h"
#include "LoggingModule.h"
#include "FileManager.h"
#include "Resource.h"
#include <string>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <experimental/filesystem>
#include <GeneralTypes.h>
#include <functional>
#include <system_error>
#include <sstream>

namespace simpleP2P {
CLI::CLI(Resource_Database &res_db_, Logging_Module &Logger_, boost::asio::io_service &io_service_,
         FileManager &fm_, Host &localhost_, Printer &printer_, Udp_Module &udp_) : res_db(res_db_), Logger(Logger_),
                                                                                    io_service(io_service_), fm(fm_),
                                                                                    localhost(localhost_),
                                                                                    printer(printer_), udp(udp_) {
  CLICommands = {
      CLICommand("add", "\"add name_of_file\" - adds local file to resource db, will be broadcasted",
                 [this](std::string name_of_file) {
                   std::error_code errc;
                   Uint64 size = std::experimental::filesystem::file_size(name_of_file, errc);

                   if (not errc) {
                     stream << " File size is: " << size << "\n";
                     print_text(stream);

                     const Resource new_resource(name_of_file, size,
                                                 name_of_file); //not sure if path is correct?
                     res_db.add_file(new_resource);
                     return 1;
                   } else if (errc == std::errc::no_such_file_or_directory) {
                     stream << " File does not exist. Are you sure it is in Simple_P2P/bin?\n";
                     print_text(stream);
                     return 0;
                   }
                   return 0;
                 }),

      CLICommand("revoke", "\"revoke name_of_file\" - invalidates a resource",
                 [this](std::string name_of_file) {
                   std::vector<std::shared_ptr<Resource>> resources = res_db.getResources();

                   for (auto resource : resources) {
                     if (resource->getName() == name_of_file) {
                       udp.revoke_file(*resource);
                       stream << " Done.\n";
                       print_text(stream);
                       return 1;
                     }
                   }
                   stream << " File does not exist. Are you sure it is in database?\n";
                   print_text(stream);

                   return 0;
                 }),
      CLICommand("download", "\"download name_of_file\" - downloads a file using p2p",
                 [this](std::string name_of_file) {
                   std::vector<std::shared_ptr<Resource>> resources = res_db.getResources();

                   for (auto resource : resources) {
                     if (resource->getName() == name_of_file) {
                       if (resource->isInvalidated()) {
                         stream << " File found, invaldiated. Cannot be downloaded.\n";
                         print_text(stream);
                         return 0;
                       }
                       stream << " File found, starting download.\n";
                       print_text(stream);
                       std::thread downloader([&]() {
                         DownloadService ds(Logger, io_service, fm, res_db, resource);
                         ds.init();
                       });
                       downloader.detach();

                       return 1;
                     }
                   }
                   stream << " File not found.\n";
                   print_text(stream);
                   return 0;
                 }),

      CLICommand("global", "prints files in the system", [this](std::string placeholder) {
        (void) placeholder;
        std::vector<std::shared_ptr<Resource>> resources = res_db.getResources();

        for (auto resource : resources) {
          stream << " " << resource->getName();
          if (localhost.has_resource(*resource))
            stream << " [owned]";
          if (resource->isInvalidated())
            stream << " [revoked]";
          stream << "\n";
        }
        print_text(stream);

        return 0;
      }),

      CLICommand("help", "prints available commands", [this](std::string placeholder) {
        (void) placeholder;
        for (auto &command : CLICommands)
          stream << command;
        print_text(stream);
        return 1;
      }),
      CLICommand("quit", "leaves the program", [](std::string x) {
        (void) x;
        exit(1);
        return 0;
      })};
}

void CLI::print_init_info() {
  stream << "-----\n";
  stream << "Simple P2P downloader - TIN 19Z project\n";
  stream << "Wiktor Michalski\n";
  stream << "Przemyslaw Stawczyk\n";
  stream << "Maciej Szulik\n";
  stream << "Kamil Zacharczuk\n";
  stream << "-----\n";
  print_text(stream);
  execute_command("help", "");
}

std::thread CLI::init() {
  return std::thread([&] { start_CLI(); });
}

void CLI::start_CLI() {
  std::string line;
  std::vector<std::string> vec;

  print_init_info();

  while (std::getline(std::cin, line)) {
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    boost::char_separator<char> sep(" ");
    tokenizer tokens(line, sep);

    vec.assign(tokens.begin(), tokens.end());
    if (vec.size() == 0)
      continue;

    if (vec.size() == 1)
      vec.push_back("");

    execute_command(vec[0], vec[1]);
  }
}

void CLI::print_text(std::stringstream &text) {
  printer.print(text.str());
  stream.str(std::string());
}

void CLI::execute_command(std::string name, std::string arg) {
  bool found = 0;
  for (auto &command : CLICommands) {

    if (name == command.getName()) {
      found = 1;
      command(arg);
    }
  }
  if (found == 0) {
    stream << "* type 'help' for the list of available commands\n";
    print_text(stream);
  }
}

CLI::~CLI() {}

} // namespace simpleP2P
