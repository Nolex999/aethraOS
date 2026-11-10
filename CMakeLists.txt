cmake_minimum_required(VERSION 3.20)
project(aethra VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(aethra_core STATIC
  src/core/ae_core.c
  src/gfx/ae_font.c
  src/gfx/ae_gfx.c
  src/boot_anim/boot_anim.c
  src/shell/ae_home.c
  src/shell/ae_system.c
)
target_include_directories(aethra_core PUBLIC src)
target_compile_options(aethra_core PRIVATE -Wall -Wextra -Werror)

if(CMAKE_CROSSCOMPILING)
  # Compile check only: the real .nwa is built by tools/build/build_nwa.sh with nwlink.
  add_library(aethra_eadk_check OBJECT src/platform/eadk/main_eadk.c)
  target_include_directories(aethra_eadk_check PRIVATE src tests/stubs)
  target_compile_options(aethra_eadk_check PRIVATE -Wall -Wextra -Werror)

  add_custom_target(size_report ALL
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:aethra_core>
    DEPENDS aethra_core
    COMMENT "Size per object (Cortex-M7)"
  )
  return()
endif()

add_library(aethra_sim_platform STATIC src/platform/sim/sim_platform.c)
target_include_directories(aethra_sim_platform PUBLIC src)
target_compile_options(aethra_sim_platform PRIVATE -Wall -Wextra -Werror)

set(SIM_SOURCES src/platform/sim/sim_main.c)
if(WIN32)
  list(APPEND SIM_SOURCES src/platform/sim/sim_win32.c)
endif()
add_executable(aethra_sim ${SIM_SOURCES})
target_compile_options(aethra_sim PRIVATE -Wall -Wextra -Werror)
target_link_libraries(aethra_sim PRIVATE aethra_core aethra_sim_platform)
if(WIN32)
  target_link_libraries(aethra_sim PRIVATE gdi32)
endif()

add_executable(boot_anim_host src/boot_anim/host/host_main.c)
target_compile_options(boot_anim_host PRIVATE -Wall -Wextra -Werror)
target_link_libraries(boot_anim_host PRIVATE aethra_core)

enable_testing()
find_package(Python3 COMPONENTS Interpreter)

foreach(name test_core test_boot_anim)
  add_executable(${name} tests/unit/${name}.c)
  target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(${name} PRIVATE aethra_core)
  add_test(NAME ${name} COMMAND ${name})
endforeach()

add_executable(test_replay tests/integration/test_replay.c)
target_compile_options(test_replay PRIVATE -Wall -Wextra -Werror)
target_link_libraries(test_replay PRIVATE aethra_core aethra_sim_platform)
add_test(NAME test_replay COMMAND test_replay ${CMAKE_SOURCE_DIR}/tests/golden/replay.txt)

if(Python3_Interpreter_FOUND)
  add_test(NAME check_layers COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/tools/analysis/check_layers.py ${CMAKE_SOURCE_DIR}/src)
  add_test(NAME test_flash COMMAND Python3::Interpreter -B ${CMAKE_SOURCE_DIR}/tests/unit/test_flash.py)
endif()
