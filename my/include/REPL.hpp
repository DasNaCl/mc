#pragma once

#include <string>

class REPL
{
public:
  void loop();

private:
  std::string parse_input();
};

