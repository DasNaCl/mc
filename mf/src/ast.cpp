#include <ast.hpp>

std::uint_fast64_t Statement::gid_counter = 0;
std::uint_fast64_t Type::gid_counter = 0;
std::uint_fast64_t Expression::gid_counter = 0;

Type::Type()
  : id(gid_counter++)
{  }

std::uint_fast64_t Type::gid() const
{ return id; }

Statement::Statement(SourceRange loc)
  : id(gid_counter++), loc(loc)
{  }

std::uint_fast64_t Statement::gid() const
{ return id; }

Expression::Expression(SourceRange loc)
  : id(gid_counter++), loc(loc)
{  }

SourceRange Expression::source_range()
{ return loc; }

std::uint_fast64_t Expression::gid() const
{ return id; }

Identifier::Identifier(SourceRange loc, Symbol symbol)
  : Statement(loc), symbol(symbol)
{  }

Declaration::Declaration(SourceRange loc, Identifier::Ptr identifier, Type::Ptr type)
  : Statement(loc), identifier(identifier), type(type)
{  }

Parameter::Parameter(SourceRange range, Identifier::Ptr identifier, Type::Ptr type)
  : Declaration(range, identifier, type)
{  }

Parameters::Parameters(SourceRange loc, const std::vector<Parameter::Ptr>& list)
  : Statement(loc), list(list)
{  }

Block::Block(SourceRange loc, const std::vector<Statement::Ptr>& statements)
  : Statement(loc), statements(statements)
{  }

Function::Function(SourceRange loc, const std::vector<Statement::Ptr>& data)
  : Statement(loc), data(data)
{  }

ExpressionStatement::ExpressionStatement(Expression::Ptr expr)
  : Statement(expr->source_range())
{  }

LiteralExpression::LiteralExpression(Token tok)
  : Expression(tok.range), kind(tok.kind), data(tok.data)
{  }

BinaryExpression::BinaryExpression(Expression::Ptr left, Expression::Ptr right)
  : Expression(left->source_range() + right->source_range()), left(left), right(right)
{  }
