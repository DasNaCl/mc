#pragma once

#include <SourceRange.hpp>
#include <tokenizer.hpp>
#include <util.hpp>
#include <memory>

class Type : public std::enable_shared_from_this<Type>
{
public:
  using Ptr = std::shared_ptr<Type>;
  
  Type();

  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;

  std::uint_fast64_t id;
};

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

class Expression : public std::enable_shared_from_this<Expression>
{
public:
  using Ptr = std::shared_ptr<Expression>;

  Expression(SourceRange loc);

  SourceRange source_range();
  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;

  std::uint_fast64_t id;
  SourceRange loc;
};

class Identifier : public Statement
{
public:
  using Ptr = std::shared_ptr<Identifier>;

  Identifier(SourceRange loc, Symbol symbol);

private:
  Symbol symbol;
};

class Declaration : public Statement
{
public:
  using Ptr = std::shared_ptr<Declaration>;

  Declaration(SourceRange range, Identifier::Ptr identifier, Type::Ptr type);
private:
  SourceRange range;
  Identifier::Ptr identifier;
  Type::Ptr type;
};

class Parameter : public Declaration
{
public:
  using Ptr = std::shared_ptr<Parameter>;

  Parameter(SourceRange range, Identifier::Ptr identifier, Type::Ptr type);
private:
  Identifier::Ptr identifier;
  Type::Ptr type;
};

class Parameters : public Statement
{
public:
  using Ptr = std::shared_ptr<Parameters>;

  Parameters(SourceRange range, const std::vector<Parameter::Ptr>& list);
private:
  std::vector<Parameter::Ptr> list;
};

class Block : public Statement
{
public:
  using Ptr = std::shared_ptr<Block>;

  Block(SourceRange range, const std::vector<Statement::Ptr>& statements);
private:
  std::vector<Statement::Ptr> statements;
};

class Function : public Statement
{
public:
  using Ptr = std::shared_ptr<Function>;

  Function(SourceRange loc, const std::vector<Statement::Ptr>& data);
private:
  std::vector<Statement::Ptr> data;
};

class ExpressionStatement : public Statement
{
public:
  using Ptr = std::shared_ptr<ExpressionStatement>;

  ExpressionStatement(Expression::Ptr expr);
private:
  Expression::Ptr expr;
};

class LiteralExpression : public Expression
{
public:
  using Ptr = std::shared_ptr<LiteralExpression>;

  LiteralExpression(Token kind);
private:
  TokenKind kind;
  void* data;
};

class BinaryExpression : public Expression
{
public:
  using Ptr = std::shared_ptr<BinaryExpression>;

  BinaryExpression(Expression::Ptr left, Expression::Ptr right);
private:
  Expression::Ptr left;
  Expression::Ptr right;
};


struct ASTVisitor
{
  virtual void visit(Statement::Ptr stmt) {  }
  virtual void visit(Identifier::Ptr id) {  }
  virtual void visit(Declaration::Ptr decl) {  }
  virtual void visit(Parameter::Ptr param) {  }
  virtual void visit(Parameters::Ptr params) {  }
  virtual void visit(Block::Ptr block) {  }
  virtual void visit(Function::Ptr fun) {  }
  virtual void visit(ExpressionStatement::Ptr expr_stmt) {  }

  virtual void visit(LiteralExpression::Ptr lit_expr) {  }
  virtual void visit(BinaryExpression::Ptr bin_expr) {  }
};

