#include <ast.hpp>

std::uint_fast64_t Statement::gid_counter = 0;

Statement::Statement(SourceRange loc)
  : id(gid_counter++), loc(loc)
{  }

std::uint_fast64_t Statement::gid() const
{
  return id;
}


Identifier::Identifier(SourceRange loc, Symbol symbol)
  : Statement(loc)
{  }

