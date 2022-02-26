#ifndef __TRUTHTABLEEXPRESSIONPARSER_HPP__
#define __TRUTHTABLEEXPRESSIONPARSER_HPP__

#include <string>
#include <queue>
#include <unordered_map>
#include "text/expression/IValueToken.hpp"
#include "text/expression/IVariableToken.hpp"
#include "text/expression/IUnaryOperatorToken.hpp"
#include "text/expression/IBinaryOperatorToken.hpp"
#include "text/expression/IFunctionToken.hpp"
#include "text/expression/ExpressionTokenizer.hpp"
#include "text/expression/ExpressionPostfixParser.hpp"
#include "text/expression/ExpressionEvaluator.hpp"
#include "text/expression/Token.hpp"

class TruthTableExpressionParser : public ExpressionTokenizer, public ExpressionPostfixParser, public ExpressionEvaluator
{
  public:
  std::queue<IToken*> Parse(const std::string& expression);
  IValueToken* Evaluate(const std::queue<IToken*>& postfix);

  void SetUnaryOperators(const std::unordered_map<char, IUnaryOperatorToken*>* value);
  void SetBinaryOperators(const std::unordered_map<std::string, IBinaryOperatorToken*>* value);
  void SetVariables(const std::unordered_map<std::string, IVariableToken*>* value);
  void SetFunctions(const std::unordered_map<std::string, IFunctionToken*>* value);

  virtual ~TruthTableExpressionParser() override = default;
  TruthTableExpressionParser();
  TruthTableExpressionParser(const TruthTableExpressionParser& other);
  TruthTableExpressionParser(TruthTableExpressionParser&& other);

  private:
  using ExpressionTokenizer::Execute;
  using ExpressionPostfixParser::Execute;
  using ExpressionEvaluator::Execute;

  const std::unordered_map<char, IUnaryOperatorToken*>* m_pUnaryOperators;
  const std::unordered_map<std::string, IBinaryOperatorToken*>* m_pBinaryOperators;
  const std::unordered_map<std::string, IVariableToken*>* m_pVariables;
  const std::unordered_map<std::string, IFunctionToken*>* m_pFunctions;
};

#endif // __TRUTHTABLEEXPRESSIONPARSER_HPP__
