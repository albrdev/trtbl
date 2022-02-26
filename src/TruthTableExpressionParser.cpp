#include "TruthTableExpressionParser.hpp"

std::queue<IToken*> TruthTableExpressionParser::Parse(const std::string& expression)
{
  auto tokens  = ExpressionTokenizer::Execute(expression, m_pUnaryOperators, m_pBinaryOperators, m_pVariables, m_pFunctions);
  auto postfix = ExpressionPostfixParser::Execute(tokens);
  return postfix;
}

IValueToken* TruthTableExpressionParser::Evaluate(const std::queue<IToken*>& postfix)
{
  auto result = ExpressionEvaluator::Execute(postfix);
  return result;
}

void TruthTableExpressionParser::SetUnaryOperators(const std::unordered_map<char, IUnaryOperatorToken*>* value) { m_pUnaryOperators = value; }
void TruthTableExpressionParser::SetBinaryOperators(const std::unordered_map<std::string, IBinaryOperatorToken*>* value) { m_pBinaryOperators = value; }
void TruthTableExpressionParser::SetVariables(const std::unordered_map<std::string, IVariableToken*>* value) { m_pVariables = value; }
void TruthTableExpressionParser::SetFunctions(const std::unordered_map<std::string, IFunctionToken*>* value) { m_pFunctions = value; }

TruthTableExpressionParser::TruthTableExpressionParser()
    : ExpressionTokenizer()
    , ExpressionPostfixParser()
    , ExpressionEvaluator()
    , m_pVariables(nullptr)
{}

TruthTableExpressionParser::TruthTableExpressionParser(const TruthTableExpressionParser& other)
    : ExpressionTokenizer(other)
    , ExpressionPostfixParser(other)
    , ExpressionEvaluator(other)
    , m_pVariables(other.m_pVariables)
{}

TruthTableExpressionParser::TruthTableExpressionParser(TruthTableExpressionParser&& other)
    : ExpressionTokenizer(std::move(other))
    , ExpressionPostfixParser(std::move(other))
    , ExpressionEvaluator(std::move(other))
    , m_pVariables(std::move(other.m_pVariables))
{}
