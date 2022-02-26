cmake_minimum_required(VERSION 3.14)

target_sources(${TARGET_TRTBL}
  PUBLIC
    TruthTableExpressionParser.hpp
    Setup.hpp

  PRIVATE
    TruthTableExpressionParser.cpp
    TruthTableSetup.cpp
)
