# Install script for directory: D:/nRF/ncs/zephyr/ext/hal

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/atmel/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/cmsis/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/cypress/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/nordic/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/nxp/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/openisa/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/st/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/ti/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/silabs/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/esp/cmake_install.cmake")
  include("D:/nRF/ncs/nrf/applications/Lenet/build_nrf9160_pca10090ns/spm/zephyr/ext/hal/microchip/cmake_install.cmake")

endif()

