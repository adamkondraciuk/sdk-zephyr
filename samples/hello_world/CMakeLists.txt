# Copyright (c) 2023 ELMODIS Ltd.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE
    src/main.c
    src/ade9000.c
    src/ade9000Estimates.c
    src/ade9000Interface.c
)
