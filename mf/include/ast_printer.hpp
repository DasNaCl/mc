#pragma once

#include <ast.hpp>

struct ASTPrinter : ASTVisitor
{
  void leave(Block::Ptr stmt) override;

  void visit(Statement::Ptr stmt) override;
  void visit(Identifier::Ptr id) override;
  void visit(Declaration::Ptr decl) override;
  void visit(Parameter::Ptr param) override;
  void visit(Parameters::Ptr params) override;
  void visit(Block::Ptr block) override;
  void visit(Function::Ptr fun) override;
  void visit(ExpressionStatement::Ptr expr_stmt) override;

  void visit(LiteralExpression::Ptr lit_expr) override;
  void visit(BinaryExpression::Ptr bin_expr) override;
private:
  std::ostream& streamout();
private:
  std::size_t depth { 0 };
};

