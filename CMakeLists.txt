cmake_minimum_required(VERSION 3.15)
project(ClayManager)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Buscar dependencias
find_package(raylib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBVLC REQUIRED libvlc)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(owl ${SOURCES})

# Incluir directorios de VLC
target_include_directories(owl PRIVATE
    ${LIBVLC_INCLUDE_DIRS}
)

# Linkear bibliotecas
target_link_libraries(owl
    raylib
    ${LIBVLC_LIBRARIES}
)

# Añadir flags de compilación de VLC si es necesario
target_compile_options(owl PRIVATE ${LIBVLC_CFLAGS_OTHER})

find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
if(NOT CLANG_TIDY_COMMAND)
  message(WARNING "clang-tidy not found. Code analysis will be skipped.")
endif()

file(COPY ${CMAKE_SOURCE_DIR}/src/resources
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
