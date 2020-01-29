#ifndef SIMPLE_P2P_CLICOMMAND_H
#define SIMPLE_P2P_CLICOMMAND_H

#include "GeneralTypes.h"
#include <string>
#include <functional>

namespace simpleP2P {

class CLICommand {
  std::string name;
  std::string description;
  std::function<Int32(const std::string &)> function;

public:
  CLICommand(std::string, std::string, std::function<Int32(const std::string &)>);

  ~CLICommand() {};

  void operator()(const std::string &) const;

  std::string getName() const { return name; };

  std::string getDesc() const { return description; };
};

std::ostream &operator<<(std::ostream &, const simpleP2P::CLICommand &);

} // namespace simpleP2P

#endif // SIMPLE_P2P_CLICOMMAND_H
