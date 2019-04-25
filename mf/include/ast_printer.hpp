#pragma once

#include <ast.hpp>

struct ASTPrinter : ASTVisitor
{
  void leave(Block::Ptr) override;
  void leave(Function::Ptr) override;
  void leave(ExpressionStatement::Ptr) override;
  void leave(FunctionCall::Ptr) override;
  void leave(LambdaExpression::Ptr) override;

  void visit(Identifier::Ptr id) override;
  void visit(Declaration::Ptr decl) override;
  void visit(Parameter::Ptr param) override;
  void visit(Parameters::Ptr params) override;
  void visit(Block::Ptr block) override;
  void visit(Function::Ptr fun) override;
  void visit(ExpressionStatement::Ptr expr_stmt) override;
  void visit(ErrorStatement::Ptr err_stmt) override;

  void visit(ErrorExpression::Ptr err_expr) override;
  void visit(LiteralExpression::Ptr lit_expr) override;
  void visit(BinaryExpression::Ptr bin_expr) override;
  void visit(FunctionCall::Ptr fn_call) override;
  void visit(LambdaExpression::Ptr lam) override;

  // inner visiting functions for types
  void visit(Unit::Ptr typ) override;
  void visit(PrimitiveType::Ptr typ) override;
  void visit(FunctionType::Ptr typ) override;
  void visit(TemplateType::Ptr typ) override;
  void visit(TupleType::Ptr typ) override;
  void visit(ArgsType::Ptr typ) override;
  void visit(ErrorType::Ptr typ) override;
private:
  std::ostream& streamout();
private:
  std::size_t depth { 0 };
};

