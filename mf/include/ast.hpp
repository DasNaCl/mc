#pragma once

#include <SourceRange.hpp>
#include <tokenizer.hpp>
#include <util.hpp>
#include <memory>

struct ASTVisitor;

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
  friend struct ASTVisitor;
  friend class Function;
  friend class Block;
public:
  using Ptr = std::shared_ptr<Statement>;

  Statement(SourceRange loc);

  std::uint_fast64_t gid() const;
protected:
  virtual void enter(ASTVisitor& vis);
  virtual void visit(ASTVisitor& vis);
  virtual void leave(ASTVisitor& vis);
private:
  static std::uint_fast64_t gid_counter;

  std::uint_fast64_t id;
  SourceRange loc;
};

class Expression : public std::enable_shared_from_this<Expression>
{
  friend struct ASTVisitor;
  friend class ExpressionStatement;
  friend class BinaryExpression;
public:
  using Ptr = std::shared_ptr<Expression>;

  Expression(SourceRange loc);

  SourceRange source_range();
  std::uint_fast64_t gid() const;
protected:
  virtual void enter(ASTVisitor& vis);
  virtual void visit(ASTVisitor& vis);
  virtual void leave(ASTVisitor& vis);
private:
  static std::uint_fast64_t gid_counter;

  std::uint_fast64_t id;
  SourceRange loc;
};

struct Unit : public Type
{

};

struct PrimitiveType : public Type
{
public:
  PrimitiveType(Symbol name);
private:
  Symbol name;
};

struct FunctionType : public Type
{
public:
  FunctionType(Type::Ptr arg_typ, Type::Ptr ret_typ);
private:
  Type::Ptr arg_typ;
  Type::Ptr ret_typ;
};

class Identifier : public Statement
{
public:
  using Ptr = std::shared_ptr<Identifier>;

  Identifier(SourceRange loc, Symbol symbol);

  Symbol id() const;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Symbol symbol;
};

class Declaration : public Statement
{
public:
  using Ptr = std::shared_ptr<Declaration>;

  Declaration(SourceRange range, Identifier::Ptr identifier, Type::Ptr type);
protected:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
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
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
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
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  std::vector<Parameter::Ptr> list;
};

class Block : public Statement
{
public:
  using Ptr = std::shared_ptr<Block>;

  Block(SourceRange range, const std::vector<Statement::Ptr>& statements);
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  std::vector<Statement::Ptr> statements;
};

class Function : public Statement
{
public:
  using Ptr = std::shared_ptr<Function>;

  Function(SourceRange loc, const std::vector<Statement::Ptr>& data, Type::Ptr ret_typ);
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  std::vector<Statement::Ptr> data;
  Type::Ptr ret_typ;
};

class ExpressionStatement : public Statement
{
public:
  using Ptr = std::shared_ptr<ExpressionStatement>;

  ExpressionStatement(Expression::Ptr expr);
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Expression::Ptr expr;
};

class LiteralExpression : public Expression
{
public:
  using Ptr = std::shared_ptr<LiteralExpression>;

  LiteralExpression(Token kind);
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
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
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Expression::Ptr left;
  Expression::Ptr right;
};


struct ASTVisitor
{
  void visit_all(const std::vector<Statement::Ptr>& ast);

  virtual void enter(Statement::Ptr stmt) {  }
  virtual void enter(Identifier::Ptr id) {  }
  virtual void enter(Declaration::Ptr decl) {  }
  virtual void enter(Parameter::Ptr param) {  }
  virtual void enter(Parameters::Ptr params) {  }
  virtual void enter(Block::Ptr block) {  }
  virtual void enter(Function::Ptr fun) {  }
  virtual void enter(ExpressionStatement::Ptr expr_stmt) {  }

  virtual void enter(Expression::Ptr expr) {  }
  virtual void enter(LiteralExpression::Ptr lit_expr) {  }
  virtual void enter(BinaryExpression::Ptr bin_expr) {  }


  virtual void visit(Statement::Ptr stmt) {  }
  virtual void visit(Identifier::Ptr id) {  }
  virtual void visit(Declaration::Ptr decl) {  }
  virtual void visit(Parameter::Ptr param) {  }
  virtual void visit(Parameters::Ptr params) {  }
  virtual void visit(Block::Ptr block) {  }
  virtual void visit(Function::Ptr fun) {  }
  virtual void visit(ExpressionStatement::Ptr expr_stmt) {  }

  virtual void visit(Expression::Ptr expr) {  }
  virtual void visit(LiteralExpression::Ptr lit_expr) {  }
  virtual void visit(BinaryExpression::Ptr bin_expr) {  }


  virtual void leave(Statement::Ptr stmt) {  }
  virtual void leave(Identifier::Ptr id) {  }
  virtual void leave(Declaration::Ptr decl) {  }
  virtual void leave(Parameter::Ptr param) {  }
  virtual void leave(Parameters::Ptr params) {  }
  virtual void leave(Block::Ptr block) {  }
  virtual void leave(Function::Ptr fun) {  }
  virtual void leave(ExpressionStatement::Ptr expr_stmt) {  }

  virtual void leave(Expression::Ptr expr) {  }
  virtual void leave(LiteralExpression::Ptr lit_expr) {  }
  virtual void leave(BinaryExpression::Ptr bin_expr) {  }
};

