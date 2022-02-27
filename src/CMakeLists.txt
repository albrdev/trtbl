cmake_minimum_required(VERSION 3.14)

target_sources(${TARGET_TRTBL}
  PUBLIC
    Setup.hpp

  PRIVATE
    TruthTableSetup.cpp
)
