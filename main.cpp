#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <regex>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include "math/Common.hpp"
#include "Setup.hpp"

static const std::unordered_map<Associativity, std::string> associativityNameMap = {
    {Associativity::Left, "Left"},
    {Associativity::Right, "Right"},
    {Associativity::Any, "Any"},
};

void resolveEnvironmentVariables(std::vector<std::string>& result)
{
  const char* pTmp;

  if((pTmp = std::getenv("TRTBL_TRUE")) != nullptr)
  {
    result.push_back("TRTBL_TRUE");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_FALSE")) != nullptr)
  {
    result.push_back("TRTBL_FALSE");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_ISEP")) != nullptr)
  {
    result.push_back("TRTBL_ISEP");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_OSEP")) != nullptr)
  {
    result.push_back("TRTBL_OSEP");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_IPAD_A")) != nullptr)
  {
    result.push_back("TRTBL_IPAD_A");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_IPAD_B")) != nullptr)
  {
    result.push_back("TRTBL_IPAD_B");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_OPAD_A")) != nullptr)
  {
    result.push_back("TRTBL_OPAD_A");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_OPAD_B")) != nullptr)
  {
    result.push_back("TRTBL_OPAD_B");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("TRTBL_JUXTA")) != nullptr)
  {
    result.push_back("TRTBL_JUXTA");
    result.push_back(pTmp);
  }
}

template<typename InputIterator, typename T>
bool cartesianProduct(InputIterator begin, InputIterator end, T min, T max)
{
  auto iter = std::find_if_not(std::make_reverse_iterator(end), std::make_reverse_iterator(begin), [&max](auto current) { return max == current; });

  if(iter == std::make_reverse_iterator(begin))
  {
    return false;
  }
  else
  {
    auto boundIter = std::prev(iter.base());
    (*boundIter)++;
    std::fill(std::next(boundIter), end, min);
    return true;
  }
}

static void assignInput(const std::list<unsigned int>& premutations)
{
  auto iter1 = defaultUninitializedVariableCache.begin();
  auto iter2 = premutations.begin();
  for(; iter1 != defaultUninitializedVariableCache.end(); iter1++, iter2++)
  {
    iter1->get()->As<DefaultVariableType*>()->SetValue(static_cast<bool>(*iter2));
  }
}

static void clearVariableCache()
{
  while(!defaultUninitializedVariableCache.empty())
  {
    const auto& top = defaultUninitializedVariableCache.front();
    defaultVariables.erase(top.get()->GetIdentifier());
    defaultUninitializedVariableCache.pop_front();
  }
}

static void evaluate(const std::string& expression, ExpressionParserBase& expressionParser)
{
  auto queue = expressionParser.Parse(expression);
  std::list<unsigned int> premutations(defaultUninitializedVariableCache.size(), 0u);
  const std::size_t maxSubLen = std::max(options.fsub.length(), options.tsub.length());
  std::list<std::size_t> columnAlignment;

  const auto last = std::prev(defaultUninitializedVariableCache.cend());
  auto iter       = defaultUninitializedVariableCache.cbegin();
  for(; iter != last; iter++)
  {
    columnAlignment.push_back(std::max(iter->get()->GetIdentifier().length(), maxSubLen));
    std::string lineFormat = (boost::format("%%|1$-%1%|") % (columnAlignment.back() + (options.ipad_a + options.ipad_b) + 1u)).str();
    std::cout << (boost::format(lineFormat) % iter->get()->GetIdentifier());
  }
  columnAlignment.push_back(std::max(iter->get()->GetIdentifier().length(), maxSubLen));
  std::cout << iter->get()->GetIdentifier() << std::endl;

  do
  {
    assignInput(premutations);
    auto result     = DefaultValueType(expressionParser.Evaluate(queue)->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
    const auto last = std::prev(premutations.cend());
    auto iter       = premutations.cbegin();
    auto alignIter  = columnAlignment.cbegin();
    for(; iter != last; iter++, alignIter++)
    {
      std::string lineFormat = (boost::format("%%|1$-%1%|%%|2$-%2%|") % (*alignIter + options.ipad_a) % (options.ipad_b + 1u)).str();
      std::cout << (boost::format(lineFormat) % ((*iter != 0u) ? options.tsub : options.fsub) % options.isep);
    }
    std::string lineFormat = (boost::format("%%|1$-%1%|%%|2$-%2%|%%|3$|") % (*alignIter + options.opad_a) % (options.opad_b + 1u)).str();
    std::cout << (boost::format(lineFormat) % ((*iter != 0u) ? options.tsub : options.fsub) % options.osep %
                  (result.GetValue<DefaultArithmeticType>() ? options.tsub : options.fsub))
              << std::endl;
  } while(cartesianProduct(premutations.begin(), premutations.end(), 0u, 1u));

  clearVariableCache();
}

static void list(const std::string& searchPattern)
{
  const std::regex regex(searchPattern);
  bool isPrevEmptyLine = true;

  std::cerr << "Unary operators" << std::endl;
  for(const auto& i : defaultUnaryOperatorInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto identifier    = std::string(1u, entry->GetIdentifier());
    const auto precedence    = entry->GetPrecedence();
    const auto associativity = associativityNameMap.at(entry->GetAssociativity());
    const auto& title        = std::get<1u>(i);
    const auto& description  = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-5|%|2$-5|%|3$-9|%|4$-7|%|5$|") % identifier % precedence % associativity % title % description) << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Binary operators" << std::endl;
  for(const auto& i : defaultBinaryOperatorInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier   = entry->GetIdentifier();
    const auto precedence    = entry->GetPrecedence();
    const auto associativity = associativityNameMap.at(entry->GetAssociativity());
    const auto& title        = std::get<1u>(i);
    const auto& description  = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-6|%|2$-5|%|3$-9|%|4$-15|%|5$|") % identifier % precedence % associativity % title % description) << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Functions" << std::endl;
  for(const auto& i : defaultFunctionInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier = entry->GetIdentifier();
    const auto argMinCount = ((entry->GetMinArgumentCount() != FunctionToken::GetArgumentCountMaxLimit()) ? std::to_string(entry->GetMinArgumentCount()) : "-");
    const auto argMaxCount = ((entry->GetMaxArgumentCount() != FunctionToken::GetArgumentCountMaxLimit()) ? std::to_string(entry->GetMaxArgumentCount()) : "-");
    const auto& title      = std::get<1u>(i);
    const auto& description = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-8|%|2$-5|%|3$-5|%|4$-7|%|5$|") % identifier % argMinCount % argMaxCount % title % description) << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Variables" << std::endl;
  for(const auto& i : defaultVariableInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier  = entry->GetIdentifier();
    const auto& title       = std::get<1u>(i);
    const auto& description = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-9|%|2$-9|%|3$|") % identifier % title % description) << std::endl;
      isPrevEmptyLine = false;
    }
  }
}

static void printOptions()
{
  std::cerr << "Options" << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "True substitution" % options.tsub) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "False substitution" % options.fsub) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Input separator" % options.isep) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Output separator" % options.osep) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Input padding (Prefix)" % options.ipad_a) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Input padding (Postfix)" % options.ipad_b) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Output padding (Prefix)" % options.opad_a) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Output padding (Postfix)" % options.opad_b) << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Juxtaposition precedence" % options.jpo_precedence) << std::endl;
  std::cerr << std::endl;
}

static void printVersion() { std::cout << (boost::format("%1% v%2%") % PROJECT_NAME % PROJECT_VERSION) << std::endl; }

static void printUsage(const boost::program_options::options_description& desc)
{
  std::cerr << (boost::format("%1% -[xtfsSpPuUjlvVh] expr...") % PROJECT_EXECUTABLE) << std::endl;
  std::cerr << desc << std::endl;
}

int main(int argc, char* argv[])
{
  std::vector<std::string> envs;
  resolveEnvironmentVariables(envs);
  boost::program_options::options_description namedEnvDescs;
  namedEnvDescs.add_options()("TRTBL_TRUE", boost::program_options::value<std::string>(&options.tsub)->default_value(defaultOptions.tsub));
  namedEnvDescs.add_options()("TRTBL_FALSE", boost::program_options::value<std::string>(&options.fsub)->default_value(defaultOptions.fsub));
  namedEnvDescs.add_options()("TRTBL_ISEP", boost::program_options::value<char>(&options.isep)->default_value(defaultOptions.isep));
  namedEnvDescs.add_options()("TRTBL_OSEP", boost::program_options::value<char>(&options.osep)->default_value(defaultOptions.osep));
  namedEnvDescs.add_options()("TRTBL_IPAD_A", boost::program_options::value<std::size_t>(&options.ipad_a)->default_value(defaultOptions.ipad_a));
  namedEnvDescs.add_options()("TRTBL_IPAD_B", boost::program_options::value<std::size_t>(&options.ipad_b)->default_value(defaultOptions.ipad_b));
  namedEnvDescs.add_options()("TRTBL_OPAD_A", boost::program_options::value<std::size_t>(&options.opad_a)->default_value(defaultOptions.opad_a));
  namedEnvDescs.add_options()("TRTBL_OPAD_B", boost::program_options::value<std::size_t>(&options.opad_b)->default_value(defaultOptions.opad_b));
  namedEnvDescs.add_options()(
      "TRTBL_JUXTA",
      boost::program_options::value<int>(&options.jpo_precedence)->default_value(defaultOptions.jpo_precedence)->notifier([](int value) {
        options.jpo_precedence = Math::Sign(value);
      }));
  boost::program_options::variables_map envVariableMap;
  boost::program_options::store(boost::program_options::command_line_parser(envs)
                                    .options(namedEnvDescs)
                                    .extra_parser([](const std::string& value) { return std::make_pair(value, std::string()); })
                                    .run(),
                                envVariableMap);
  boost::program_options::notify(envVariableMap);

  boost::program_options::options_description namedArgDescs("Options");
  namedArgDescs.add_options()("expr,x", boost::program_options::value<std::vector<std::string>>(), "Add an expression");
  namedArgDescs.add_options()("true,t", boost::program_options::value<std::string>(&options.tsub), "Set \'true\' substitution");
  namedArgDescs.add_options()("false,f", boost::program_options::value<std::string>(&options.fsub), "Set \'false\' substitution");
  namedArgDescs.add_options()("isep,s", boost::program_options::value<char>(&options.isep), "Set input separator");
  namedArgDescs.add_options()("osep,S", boost::program_options::value<char>(&options.osep), "Set output separator");
  namedArgDescs.add_options()("ipad_a,p", boost::program_options::value<std::size_t>(&options.ipad_a), "Set input padding (Prefix)");
  namedArgDescs.add_options()("ipad_b,P", boost::program_options::value<std::size_t>(&options.ipad_b), "Set input padding (Postfix)");
  namedArgDescs.add_options()("opad_a,u", boost::program_options::value<std::size_t>(&options.opad_a), "Set output padding (Prefix)");
  namedArgDescs.add_options()("opad_b,U", boost::program_options::value<std::size_t>(&options.opad_b), "Set output padding (Postfix)");
  namedArgDescs.add_options()("juxta,j",
                              boost::program_options::value<int>()->notifier([](int value) { options.jpo_precedence = Math::Sign(value); }),
                              "Set juxtaposition operator precedence (-1, 0, 1)");
  namedArgDescs.add_options()("list,l", boost::program_options::value<std::string>()->implicit_value(".*"), "List available operators/variables");
  namedArgDescs.add_options()("verbose,v", "Enable verbose mode");
  namedArgDescs.add_options()("version,V", "Print version");
  namedArgDescs.add_options()("help,h", "Print usage");
  boost::program_options::positional_options_description positionalArgDescs;
  positionalArgDescs.add("expr", -1);
  boost::program_options::variables_map argVariableMap;
  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(namedArgDescs).positional(positionalArgDescs).run(),
                                argVariableMap);
  boost::program_options::notify(argVariableMap);

  if(argVariableMap.count("help") > 0u)
  {
    printUsage(namedArgDescs);
    std::exit(EXIT_SUCCESS);
  }

  if(argVariableMap.count("version") > 0u)
  {
    printVersion();
    std::exit(EXIT_SUCCESS);
  }

  if(argVariableMap.count("verbose") > 0u)
  {
    printOptions();
  }

  ExpressionParserBase expressionParser;
  InitTruthTable(expressionParser);

  if(argVariableMap.count("list") > 0u)
  {
    list(argVariableMap["list"].as<const std::string&>());
    std::exit(EXIT_SUCCESS);
  }

  bool hasPipedData = std::cin.rdbuf()->in_avail() != -1 && isatty(fileno(stdin)) == 0;
  if(hasPipedData)
  {
    std::string input;
    while(std::getline(std::cin, input))
    {
      evaluate(input, expressionParser);
    }
  }

  if(argVariableMap.count("expr") == 0u && !hasPipedData)
  {
    std::cerr << "*** Error: No expression specified" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  if(argVariableMap.count("expr") > 0u)
  {
    const auto& exprs = argVariableMap["expr"].as<const std::vector<std::string>&>();
    for(auto& expr : exprs)
    {
      evaluate(expr, expressionParser);
    }
  }

  std::exit(EXIT_SUCCESS);
}
