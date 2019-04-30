#pragma once

#include <memory>

class Statement;

enum class CallingConvention
{
  RealReduction,
  NormalOrder,
  CallByName,
  CallByValue
};

std::shared_ptr<Statement> execute(std::shared_ptr<Statement> root);

