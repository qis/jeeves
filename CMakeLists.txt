cmake_minimum_required(VERSION 3.8.1 FATAL_ERROR)
project(jeeves VERSION 0.1.0 LANGUAGES CXX)

# Project
set(PROJECT "Jeeves")
set(COMPANY "Xiphos")
set(COPYING "2017 Alexej Harm")

# Configuration
configure_file(res/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Compiler Settings
if(MSVC)
  foreach(type ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${type} type)
    foreach(flag CMAKE_C_FLAGS_${type} CMAKE_CXX_FLAGS_${type})
      string(REPLACE "/MD" "/MT" ${flag} "${${flag}}")
    endforeach()
  endforeach()
  set(CMAKE_CXX_FLAGS "/permissive- /std:c++latest ${CMAKE_CXX_FLAGS} /utf-8")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifestuac:NO /ignore:4099 /ignore:4098")
  add_definitions(-D_UNICODE -DUNICODDE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_ATL_SECURE_NO_DEPRECATE)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00)
else()
  if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(CMAKE_CXX_FLAGS "-I/opt/llvm/include/c++/v1 -nostdinc++ ${CMAKE_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "-L/opt/llvm/lib ${CMAKE_EXE_LINKER_FLAGS}")
  endif()
  set(CMAKE_CXX_FLAGS "-std=c++1z -stdlib=libc++ ${CMAKE_CXX_FLAGS} -pthread -fno-omit-frame-pointer -fcolor-diagnostics")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Werror -Wno-unused-command-line-argument -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable")
endif()

# Output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Packages
file(GLOB CMAKE_PREFIX_PATH LIST_DIRECTORIES true third_party/*)
find_package(compat REQUIRED)

# CSGO Sources
file(GLOB_RECURSE src src/csgo/*.h src/csgo/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/csgo PREFIX src FILES ${src})

# CSGO
add_library(csgo STATIC ${src})
target_include_directories(csgo PRIVATE ${CMAKE_CURRENT_BINARY_DIR} PUBLIC src)
target_compile_options(csgo PUBLIC /Zc:strictStrings-)
target_link_libraries(csgo PRIVATE compat)

# Code Soureces
file(GLOB_RECURSE src src/code/*.h src/code/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/code PREFIX src FILES ${src})

# Code Documents
if(MSVC)
  file(GLOB_RECURSE doc doc/*.md)
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/doc PREFIX doc FILES ${doc})
  list(APPEND src ${doc})
endif()

# Code
add_library(code SHARED ${src})
target_include_directories(code PRIVATE ${CMAKE_CURRENT_BINARY_DIR} PUBLIC src)
target_link_libraries(code PRIVATE compat csgo)

# Jeeves Soureces
file(GLOB src src/*.h src/*.cpp src/os/*.h src/os/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${src})

# Jeeves Resources
if(MSVC)
  file(GLOB res res/resource.h res/resource.rc res/resource.manifest)
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/res PREFIX res FILES ${res})
  list(APPEND src ${res})
endif()

# Jeeves
add_executable(${PROJECT_NAME} WIN32 ${src})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} PUBLIC src)
target_link_libraries(${PROJECT_NAME} PRIVATE compat code)

# Install
install(TARGETS code ${PROJECT_NAME} DESTINATION bin)

# Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER build)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})