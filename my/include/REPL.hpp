#pragma once

#include <memory>
#include <string>
#include <vector>

class Statement;

class REPL
{
public:
  void loop();

private:
  std::vector<std::shared_ptr<Statement>> parse_input();
};

