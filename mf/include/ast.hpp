#pragma once

#include <memory>

class Statement
{
public:
  using Ptr = std::unique_ptr<Statement>;
};

