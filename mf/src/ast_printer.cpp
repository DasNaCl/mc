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

void ASTPrinter::visit(ErrorStatement::Ptr err_stmt)
{
  streamout() << "[ErrorStatement <" << err_stmt->gid() << "> {";
  distribute(err_stmt->type());
  std::cout << "}]\n";
}

void ASTPrinter::visit(Identifier::Ptr stmt)
{
  streamout() << "[Identifier <" << stmt->gid()  << "> \"" << stmt->id() << "\" {";
  distribute(stmt->type());
  std::cout << "}]\n";
}

void ASTPrinter::visit(Declaration::Ptr decl)
{
  streamout() << "[Declaration <" << decl->gid() << "> {";
  distribute(decl->type());
  std::cout << "]\n";
}

void ASTPrinter::visit(Parameter::Ptr param)
{
  streamout() << "[Param <" << param->gid() << "> {";
  distribute(param->type());
  std::cout << "]\n";
}

void ASTPrinter::visit(Parameters::Ptr params)
{
  streamout() << "[Parameters <" << params->gid() << "> {";
  distribute(params->type());
  std::cout << "}]\n";
}

void ASTPrinter::visit(Block::Ptr block)
{
  streamout() << "[Block <" << block->gid() << "> {";
  distribute(block->type());
  std::cout << "}]\n";
  ++depth;
}

void ASTPrinter::visit(Function::Ptr fun)
{
  streamout() << "[Function <" << fun->gid() << "> {";
  distribute(fun->type());
  std::cout << "}]\n";
  ++depth;
}

void ASTPrinter::visit(ExpressionStatement::Ptr expr_stmt)
{
  streamout() << "[ExpressionStatement <" << expr_stmt->gid() << "> {";
  distribute(expr_stmt->type());
  std::cout << "}]\n";
  ++depth;
}

void ASTPrinter::visit(ErrorExpression::Ptr err_expr)
{
  streamout() << "(Error <" << err_expr->gid() << "> {";
  distribute(err_expr->type());
  std::cout << "})\n";
}

void ASTPrinter::visit(LiteralExpression::Ptr lit_expr)
{
  streamout() << "(Literal <" << lit_expr->gid() << "> {";
  distribute(lit_expr->type());
  std::cout << "})\n";
}

void ASTPrinter::visit(BinaryExpression::Ptr bin_expr)
{
  streamout() << "(Binary <" << bin_expr->gid() << "> {";
  distribute(bin_expr->type());
  std::cout << "})\n";
}

void ASTPrinter::visit(Unit::Ptr type)
{
  std::cout << "()";
}

void ASTPrinter::visit(PrimitiveType::Ptr type)
{
  std::cout << type->symbol();
}

void ASTPrinter::visit(FunctionType::Ptr type)
{
  // TODO
}

void ASTPrinter::visit(TemplateType::Ptr type)
{
  std::cout << "Polymorphic";
}

void ASTPrinter::visit(TupleType::Ptr type)
{
  auto v = type->types();
  std::cout << "(";
  for(auto it = v.begin(); it != v.end(); ++it)
  {
    auto t = *it;
    distribute(t);
    if(std::next(it) != v.end())
      std::cout << ", ";
  }
  std::cout << ")";
}

void ASTPrinter::visit(ArgsType::Ptr type)
{
  auto v = type->types();
  for(auto& var : v)
  {
    if(std::holds_alternative<ArgsType::_Id>(var))
      std::cout << "-";
    else
      distribute(std::get<Type::Ptr>(var));
  }
}

void ASTPrinter::visit(ErrorType::Ptr type)
{
  std::cout << "(ErrorType)";
}

