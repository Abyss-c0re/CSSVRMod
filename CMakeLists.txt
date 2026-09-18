cmake_minimum_required(VERSION 3.14)
project(CSSVRMod LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CSSVR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(CSSVR_INSTALL "${CSSVR_ROOT}/install")

add_library(cssvrmod_core STATIC
  src/weapons.cpp
  src/launch.cpp
  src/source_if.cpp
  src/calib.cpp
  src/usercmd.cpp
  src/settings.cpp
  src/toast.cpp
)
target_include_directories(cssvrmod_core PUBLIC
  ${CSSVR_ROOT}/include
)
target_link_libraries(cssvrmod_core PUBLIC dl)

add_executable(cssvrmod_tests
  tests/test_main.cpp
  tests/test_hand_bullet.cpp
  tests/test_melee.cpp
  tests/test_collision.cpp
  tests/test_aim.cpp
  tests/test_weapons.cpp
  tests/test_input.cpp
  tests/test_launch.cpp
  tests/test_tick.cpp
  tests/test_calib.cpp
  tests/test_stereo_view.cpp
  tests/test_menu3d.cpp
  tests/test_view_hook.cpp
  tests/test_vk_eye.cpp
  tests/test_look.cpp
  tests/test_usercmd.cpp
  tests/test_engine_trace.cpp
  tests/test_settings.cpp
  tests/test_toast.cpp
  tests/test_banner.cpp
  tests/test_xr_session.cpp
  tests/test_console.cpp
)
target_include_directories(cssvrmod_tests PRIVATE
  ${CSSVR_ROOT}/include
  ${CSSVR_ROOT}/tests
)
target_link_libraries(cssvrmod_tests PRIVATE cssvrmod_core)
set_target_properties(cssvrmod_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CSSVR_ROOT}/build"
)

add_executable(CSSVR src/app_main.cpp)
target_include_directories(CSSVR PRIVATE ${CSSVR_ROOT}/include)
target_link_libraries(CSSVR PRIVATE cssvrmod_core)
set_target_properties(CSSVR PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CSSVR_INSTALL}"
)
add_custom_command(TARGET CSSVR POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CSSVR_INSTALL}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CSSVR_ROOT}/cfg/cssvrmod.cfg" "${CSSVR_INSTALL}/cssvrmod.cfg"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CSSVR_ROOT}/cfg/cssvr_calib.cfg" "${CSSVR_INSTALL}/cssvr_calib.cfg"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CSSVR_ROOT}/scripts/CSSVR.sh" "${CSSVR_INSTALL}/CSSVR.sh"
  COMMENT "Install cfg + CSSVR.sh next to launcher"
)

option(CSSVR_BUILD_HOOK "Build libcssvrmod_hook.so" ON)
if(CSSVR_BUILD_HOOK AND NOT WIN32)
  find_library(GL_LIB GL)
  find_library(GLX_LIB GLX)
  find_package(X11)
  find_path(OPENXR_INCLUDE_DIR openxr/openxr.h)
  # Headers only — do not NEEDED libopenxr_loader (Steam PV cannot resolve it).
  if(OPENXR_INCLUDE_DIR AND GL_LIB AND GLX_LIB AND X11_FOUND)
    add_library(cssvrmod_hook SHARED
      src/hook_ctor.cpp
      src/hook_gl.cpp
      src/hook_window.cpp
      src/hook_d3d9.cpp
      src/hook_vk.cpp
      src/togl_adapter.cpp
      src/xr_host.cpp
      src/view_hook.cpp
      src/create_move.cpp
      src/cssvr_ctl.cpp
    )
    target_include_directories(cssvrmod_hook PRIVATE
      ${CSSVR_ROOT}/include
      ${CSSVR_ROOT}/src
      ${OPENXR_INCLUDE_DIR}
    )
    # X11 + Xext only. FindX11's X11_LIBRARIES also pulls SM/ICE, which Steam
    # pressure-vessel does not put on the search path (not in overrides / cache).
    target_link_libraries(cssvrmod_hook PRIVATE
      cssvrmod_core
      ${GL_LIB}
      ${GLX_LIB}
      ${X11_X11_LIB}
      ${X11_Xext_LIB}
      dl
    )
    target_compile_options(cssvrmod_hook PRIVATE -O2 -Wall -fvisibility=default)
    set_target_properties(cssvrmod_hook PROPERTIES
      OUTPUT_NAME "cssvrmod_hook"
      PREFIX "lib"
      LIBRARY_OUTPUT_DIRECTORY "${CSSVR_INSTALL}"
      BUILD_RPATH "\$ORIGIN"
      INSTALL_RPATH "\$ORIGIN"
    )
  else()
    message(WARNING "CSSVRMod: hook skipped (need OpenXR + GL + GLX + X11)")
  endif()
endif()

add_library(cssvrmod_plugin SHARED src/plugin.cpp)
target_include_directories(cssvrmod_plugin PRIVATE ${CSSVR_ROOT}/include)
target_link_libraries(cssvrmod_plugin PRIVATE dl)
set_target_properties(cssvrmod_plugin PROPERTIES
  OUTPUT_NAME "cssvrmod_plugin"
  PREFIX ""
  LIBRARY_OUTPUT_DIRECTORY "${CSSVR_INSTALL}"
)
