#include <ast_printer.hpp>
#include <iostream>

std::ostream& ASTPrinter::streamout()
{
  for(std::size_t d = depth; d > 0; --d)
  { std::cout << "  "; }

  return std::cout;
}

void ASTPrinter::leave(Block::Ptr)
{ if(depth > 0) --depth; }

void ASTPrinter::leave(Function::Ptr)
{ if(depth > 0) --depth; }

void ASTPrinter::leave(ExpressionStatement::Ptr)
{ if(depth > 0) --depth; }

void ASTPrinter::visit(Identifier::Ptr stmt)
{
  streamout() << "[Identifier <" << stmt->gid()  << "> \"" << stmt->id() << "\"]\n";
}

void ASTPrinter::visit(Declaration::Ptr decl)
{
  streamout() << "[Declaration <" << decl->gid() << ">]\n";
}

void ASTPrinter::visit(Parameter::Ptr param)
{
  streamout() << "[Param <" << param->gid() << ">]\n";
}

void ASTPrinter::visit(Parameters::Ptr params)
{
  streamout() << "[Parameters <" << params->gid() << ">]\n";
}

void ASTPrinter::visit(Block::Ptr block)
{
  streamout() << "[Block <" << block->gid() << ">]\n";
  ++depth;
}

void ASTPrinter::visit(Function::Ptr fun)
{
  streamout() << "[Function <" << fun->gid() << ">]\n";
  ++depth;
}

void ASTPrinter::visit(ExpressionStatement::Ptr expr_stmt)
{
  streamout() << "[ExpressionStatement <" << expr_stmt->gid() << ">]\n";
  ++depth;
}

void ASTPrinter::visit(LiteralExpression::Ptr lit_expr)
{
  streamout() << "(Literal <" << lit_expr->gid() << ">)\n";
}

void ASTPrinter::visit(BinaryExpression::Ptr bin_expr)
{
  streamout() << "(Binary <" << bin_expr->gid() << ">)\n";
}


