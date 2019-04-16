#pragma once

#include <ast.hpp>

struct ASTPrinter : ASTVisitor
{
  void leave(Block::Ptr) override;
  void leave(Function::Ptr) override;
  void leave(ExpressionStatement::Ptr) override;

  void visit(Identifier::Ptr id) override;
  void visit(Declaration::Ptr decl) override;
  void visit(Parameter::Ptr param) override;
  void visit(Parameters::Ptr params) override;
  void visit(Block::Ptr block) override;
  void visit(Function::Ptr fun) override;
  void visit(ExpressionStatement::Ptr expr_stmt) override;

  void visit(LiteralExpression::Ptr lit_expr) override;
  void visit(BinaryExpression::Ptr bin_expr) override;

  // inner visiting functions for types
  void acquaint(Unit::Ptr typ);
  void acquaint(PrimitiveType::Ptr typ);
  void acquaint(FunctionType::Ptr typ);
  void acquaint(TemplateType::Ptr typ);
  void acquaint(TupleType::Ptr typ);
  void acquaint(ArgsType::Ptr typ);
private:
  std::ostream& streamout();
private:
  std::size_t depth { 0 };
};

