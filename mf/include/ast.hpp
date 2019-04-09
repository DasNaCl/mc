#pragma once

#include <SourceRange.hpp>
#include <util.hpp>
#include <memory>

class Statement : public std::enable_shared_from_this<Statement>
{
public:
  using Ptr = std::shared_ptr<Statement>;

  Statement(SourceRange loc);

  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;

  std::uint_fast64_t id;
  SourceRange loc;
};

class Identifier : public Statement
{
public:
  Identifier(SourceRange loc, Symbol symbol);

private:
  Symbol symbol;
};


