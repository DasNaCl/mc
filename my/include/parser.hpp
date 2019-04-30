#pragma once

#include <ast.hpp>
#include <vector>

class Tokenizer;

std::vector<Statement::Ptr> parse(Tokenizer& tokenizer);

