#pragma once

#include <SourceRange.hpp>
#include <tokenizer.hpp>
#include <util.hpp>
#include <variant>
#include <memory>

struct ASTVisitor;

struct GIDTag
{
  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;
  std::uint_fast64_t gid_val { gid_counter++ };
};

class Type : public GIDTag, public std::enable_shared_from_this<Type>
{
  friend struct ASTVisitor;
public:
  using Ptr = std::shared_ptr<Type>;
protected:
  virtual void visit(ASTVisitor& vis); 
};

class Statement : public GIDTag, public std::enable_shared_from_this<Statement>
{
  friend struct ASTVisitor;
  friend class Function;
  friend class Block;
public:
  using Ptr = std::shared_ptr<Statement>;

  Statement(SourceRange loc);

  virtual Type::Ptr type() = 0;
protected:
  virtual void enter(ASTVisitor& vis);
  virtual void visit(ASTVisitor& vis);
  virtual void leave(ASTVisitor& vis);
private:
  SourceRange loc;
};

class Expression : public GIDTag, public std::enable_shared_from_this<Expression>
{
  friend struct ASTVisitor;
  friend class ExpressionStatement;
  friend class BinaryExpression;
public:
  using Ptr = std::shared_ptr<Expression>;

  Expression(SourceRange loc);

  SourceRange source_range();

  virtual Type::Ptr type() = 0;
protected:
  virtual void enter(ASTVisitor& vis);
  virtual void visit(ASTVisitor& vis);
  virtual void leave(ASTVisitor& vis);
private:
  SourceRange loc;
};

struct Unit : public Type
{
  friend struct ASTVisitor;
  using Ptr = std::shared_ptr<Unit>;

private:
  void visit(ASTVisitor& vis) override;
};

struct PrimitiveType : public Type
{
  friend struct ASTVisitor;
public:
  using Ptr = std::shared_ptr<PrimitiveType>;

  PrimitiveType(Symbol name);

  Symbol symbol() const;
private:
  void visit(ASTVisitor& vis) override;
private:
  Symbol name;
};

struct FunctionType : public Type
{
  friend struct ASTVisitor;
public:
  using Ptr = std::shared_ptr<FunctionType>;

  FunctionType(Type::Ptr arg_typ, Type::Ptr ret_typ);
private:
  void visit(ASTVisitor& vis) override;
private:
  Type::Ptr arg_typ;
  Type::Ptr ret_typ;
};

struct TemplateType : public Type
{
  friend struct ASTVisitor;
  using Ptr = std::shared_ptr<TemplateType>;

private:
  void visit(ASTVisitor& vis) override;
};

struct TupleType : public Type
{
  friend struct ASTVisitor;
public:
  using Ptr = std::shared_ptr<TupleType>;

  TupleType(const std::vector<Type::Ptr>& types);

  const std::vector<Type::Ptr>& types() const;
private:
  void visit(ASTVisitor& vis) override;
private:
  std::vector<Type::Ptr> data;
};

struct ArgsType : public Type
{
  friend struct ASTVisitor;
public:
  struct _Id {};

  using Ptr = std::shared_ptr<ArgsType>;

  ArgsType(const std::vector<std::variant<_Id, Type::Ptr>>& types);

  const std::vector<std::variant<_Id, Type::Ptr>>& types() const;
private:
  void visit(ASTVisitor& vis) override;
private:
  std::vector<std::variant<_Id, Type::Ptr>> data;
};

struct ErrorType : public Type
{
  friend struct ASTVisitor;
  using Ptr = std::shared_ptr<ErrorType>;
};

class ErrorStatement : public Statement
{
public:
  using Ptr = std::shared_ptr<ErrorStatement>;

  ErrorStatement(SourceRange loc);

  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
};

class Identifier : public Statement
{
public:
  using Ptr = std::shared_ptr<Identifier>;

  Identifier(SourceRange loc, Symbol symbol);

  Symbol id() const;

  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Symbol symbol;
  Type::Ptr typ;
};

class Declaration : public Statement
{
public:
  using Ptr = std::shared_ptr<Declaration>;

  Declaration(SourceRange range, Identifier::Ptr identifier, Type::Ptr type);

  Type::Ptr type() override;
protected:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  SourceRange range;
  Identifier::Ptr identifier;
  Type::Ptr typ;
};

class Parameter : public Declaration
{
public:
  using Ptr = std::shared_ptr<Parameter>;

  Parameter(SourceRange range, Identifier::Ptr identifier, Type::Ptr type);

  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Identifier::Ptr identifier;
  Type::Ptr typ;
};

class Parameters : public Statement
{
public:
  using Ptr = std::shared_ptr<Parameters>;

  Parameters(SourceRange range, const std::vector<Parameter::Ptr>& list);

  Type::Ptr type() override;
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

  Type::Ptr type() override;
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

  Type::Ptr type() override;
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

  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Expression::Ptr expr;
};

class ErrorExpression : public Expression
{
public:
  using Ptr = std::shared_ptr<ErrorExpression>;

  ErrorExpression(SourceRange loc);

  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
};

class LiteralExpression : public Expression
{
public:
  using Ptr = std::shared_ptr<LiteralExpression>;

  LiteralExpression(Token kind);

  Type::Ptr type() override;
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

  Type::Ptr type() override;
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
  
  void distribute(Statement::Ptr type);
  void distribute(Expression::Ptr type);
  void distribute(Type::Ptr type);

  virtual void enter(Statement::Ptr stmt) {  }
  virtual void enter(ErrorStatement::Ptr err) {  }
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
  virtual void visit(ErrorStatement::Ptr err) {  }
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
  virtual void leave(ErrorStatement::Ptr err) {  }
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


  virtual void visit(Type::Ptr type) {  }
  virtual void visit(ErrorType::Ptr type) {  }
  virtual void visit(Unit::Ptr type) {  }
  virtual void visit(PrimitiveType::Ptr type) {  }
  virtual void visit(FunctionType::Ptr type) {  }
  virtual void visit(TemplateType::Ptr type) {  }
  virtual void visit(TupleType::Ptr type) {  }
  virtual void visit(ArgsType::Ptr type) {  }
};

