cmake_minimum_required(VERSION 3.16)
project(SMLParser LANGUAGES CXX)

add_library(SMLParser STATIC
    src/sml_parser.cpp
)

target_include_directories(SMLParser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(SMLParser PUBLIC cxx_std_11)
