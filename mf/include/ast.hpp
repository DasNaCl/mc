#pragma once

#include <SourceRange.hpp>
#include <tokenizer.hpp>
#include <scope.hpp>
#include <util.hpp>
#include <variant>
#include <memory>

#include <tsl/hopscotch_map.h>
#include <tsl/hopscotch_set.h>

struct ASTVisitor;

struct GIDTag
{
  std::uint_fast64_t gid() const;
private:
  static std::uint_fast64_t gid_counter;
  std::uint_fast64_t gid_val { gid_counter++ };
};
struct GIDTagHasher
{
  std::size_t operator()(GIDTag* gidtag) const
  { if(gidtag) return gidtag->gid(); return 0; } // gid is unique
};
struct GIDTagComparer
{
  bool operator()(GIDTag* lhs, GIDTag* rhs) const
  { if(lhs && rhs) return lhs->gid() == rhs->gid(); return false; }
};

template<class T, 
         unsigned int NeighborhoodSize = 62,
         bool StoreHash = false,
         class GrowthPolicy = tsl::hh::power_of_two_growth_policy<2>>
using ASTNodeMap = tsl::hopscotch_map<GIDTag*, T, GIDTagHasher, GIDTagComparer, std::allocator<std::pair<GIDTag*, T>>,
                                      NeighborhoodSize, StoreHash, GrowthPolicy>;


static constexpr std::uint_fast32_t prim_types[] = { hash_string("byte"),
                                                   hash_string("char"),  hash_string("uchar"),
                                                   hash_string("short"), hash_string("ushort"),
                                                   hash_string("int"),   hash_string("uint"),
                                                   hash_string("long"),  hash_string("ulong") };
static constexpr std::size_t prim_types_len = sizeof(prim_types) / sizeof(prim_types[0]);

class Type : public GIDTag, public std::enable_shared_from_this<Type>
{
  friend struct ASTVisitor;
public:
  using Ptr = std::shared_ptr<Type>;

  Type(std::uint_fast32_t hash);

  std::uint_fast32_t shared_id() const;
protected:
  virtual void visit(ASTVisitor& vis) = 0;
private:
  static void remember(std::uint_fast32_t hash);
private:
  thread_local static tsl::hopscotch_set<std::uint_fast32_t> types;

  std::uint_fast32_t hash;
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
  friend class FunctionCall;
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
public:
  friend struct ASTVisitor;
  using Ptr = std::shared_ptr<Unit>;
  
  Unit();

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

  Type::Ptr parameter_type() const;
  Type::Ptr return_type() const;
private:
  void visit(ASTVisitor& vis) override;
private:
  Type::Ptr arg_typ;
  Type::Ptr ret_typ;
};

struct TemplateType : public Type
{
public:
  friend struct ASTVisitor;
  using Ptr = std::shared_ptr<TemplateType>;

  TemplateType();

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
public:
  friend struct ASTVisitor;
  using Ptr = std::shared_ptr<ErrorType>;

  ErrorType();

private:
  void visit(ASTVisitor& vis) override;
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

  Declaration(SourceRange range, Statement::Ptr identifier, Type::Ptr type);

  Identifier::Ptr identifier();
  Type::Ptr type() override;
protected:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  SourceRange range;
  Statement::Ptr _identifier;
  Type::Ptr typ;
};

class Parameter : public Declaration
{
public:
  using Ptr = std::shared_ptr<Parameter>;

  Parameter(SourceRange range, Statement::Ptr identifier, Type::Ptr type);
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
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

  Block(SourceRange range, Scope::Ptr scope, const std::vector<Statement::Ptr>& statements);

  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Scope::Ptr scope;

  std::vector<Statement::Ptr> statements;
};

class Function : public Statement
{
public:
  using Ptr = std::shared_ptr<Function>;

  Function(SourceRange loc, Scope::Ptr scope, const std::vector<Statement::Ptr>& data, Type::Ptr ret_typ);

  Scope& get_scope();

  std::vector<std::variant<Identifier::Ptr, Type::Ptr>> signature();

  std::vector<Identifier::Ptr> ids();
  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  Scope::Ptr scope;

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

  void* data();
  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  TokenKind kind;
  void* dat;
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

class FunctionCall : public Expression
{
public:
  using Ptr = std::shared_ptr<FunctionCall>;

  FunctionCall(SourceRange range, const std::vector<Expression::Ptr>& arguments, Symbol function_name);

  Type::Ptr type() override;
private:
  void enter(ASTVisitor& vis) override;
  void visit(ASTVisitor& vis) override;
  void leave(ASTVisitor& vis) override;
private:
  std::vector<Expression::Ptr> args;
  Symbol function_name;
};

class UnitExpression : public Expression
{
public:
  using Ptr = std::shared_ptr<UnitExpression>;

  UnitExpression(SourceRange range);

  Type::Ptr type() override;
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
  virtual void enter(ErrorExpression::Ptr err_expr) {  }
  virtual void enter(LiteralExpression::Ptr lit_expr) {  }
  virtual void enter(BinaryExpression::Ptr bin_expr) {  }
  virtual void enter(FunctionCall::Ptr fn_call) {  }


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
  virtual void visit(ErrorExpression::Ptr err_expr) {  }
  virtual void visit(LiteralExpression::Ptr lit_expr) {  }
  virtual void visit(BinaryExpression::Ptr bin_expr) {  }
  virtual void visit(FunctionCall::Ptr fn_call) {  }


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
  virtual void leave(ErrorExpression::Ptr err_expr) {  }
  virtual void leave(LiteralExpression::Ptr lit_expr) {  }
  virtual void leave(BinaryExpression::Ptr bin_expr) {  }
  virtual void leave(FunctionCall::Ptr fn_call) {  }


  virtual void visit(Type::Ptr type) {  }
  virtual void visit(ErrorType::Ptr type) {  }
  virtual void visit(Unit::Ptr type) {  }
  virtual void visit(PrimitiveType::Ptr type) {  }
  virtual void visit(FunctionType::Ptr type) {  }
  virtual void visit(TemplateType::Ptr type) {  }
  virtual void visit(TupleType::Ptr type) {  }
  virtual void visit(ArgsType::Ptr type) {  }
};

