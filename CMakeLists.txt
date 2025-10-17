# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.15)

project(g2chip 
    LANGUAGES C
    VERSION 0.0.1
    DESCRIPTION "CHIP-8, SCHIP, XO-CHIP implementation from G2Labs"
)

add_library(${PROJECT_NAME})

add_subdirectory(src)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(examples)