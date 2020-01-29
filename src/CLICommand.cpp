#ifndef SIMPLE_P2P_CLI_H
#define SIMPLE_P2P_CLI_H

#include "CLICommand.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <utility>

namespace simpleP2P {
CLICommand::CLICommand(std::string name_, std::string description_,
                       std::function<Int32(const std::string &)> function_) {
  name = std::move(name_);
  description = std::move(description_);
  function = std::move(function_);
}

void CLICommand::operator()(const std::string &argument) const {
  if (function != nullptr)
    function(argument);
  else
    std::cout << "not implemented yet :(\n";
}

std::ostream &operator<<(std::ostream &os, const CLICommand &command) {
  os << "- " << std::setw(10) << std::left << command.getName() << " " << command.getDesc() << '\n';
  return os;
}

} // namespace simpleP2P

#endif // SIMPLE_P2P_CLI_H