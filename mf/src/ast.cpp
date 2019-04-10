#include <ast.hpp>

std::uint_fast64_t Statement::gid_counter = 0;
std::uint_fast64_t Type::gid_counter = 0;

Type::Type()
  : id(gid_counter++)
{  }

std::uint_fast64_t Type::gid() const
{
  return id;
}

Statement::Statement(SourceRange loc)
  : id(gid_counter++), loc(loc)
{  }

std::uint_fast64_t Statement::gid() const
{
  return id;
}


Identifier::Identifier(SourceRange loc, Symbol symbol)
  : Statement(loc), symbol(symbol)
{  }

Var::Var(SourceRange loc, Identifier::Ptr identifier, Type::Ptr type)
  : Statement(loc), identifier(identifier), type(type)
{  }

Block::Block(SourceRange loc, const std::vector<Statement::Ptr>& statements)
  : Statement(loc), statements(statements)
{  }

Function::Function(SourceRange loc, const std::vector<Statement::Ptr>& data)
  : Statement(loc), data(data)
{  }


