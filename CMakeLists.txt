#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
#

cmake_minimum_required(VERSION 3.8.2)

include(../../cmake/boilerplate.cmake)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

# NORDIC SDK APP START
target_sources(app PRIVATE
src/main.c
#src/SD.c
src/SdFat.c
)
# NORDIC SDK APP END
zephyr_include_directories(src)

#include_directories(src/orientation_detector)
target_sources(app PRIVATE src/SD.c)
