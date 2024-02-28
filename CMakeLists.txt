cmake_minimum_required(VERSION 3.0)

project(psp2wpp_remote_conf C)

set(CMAKE_C_COMPILE_FEATURES "${CMAKE_C_FLAGS} -Wunused-result -Wl,-q -Wall -O3 -fno-inline -fno-builtin")

enable_language(CXX ASM)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb
  ${CMAKE_CURRENT_SOURCE_DIR}/libusb-compat/libusb
)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/psp2wpp.c
  src/imgui/imgui.cpp
  src/imgui/imgui_draw.cpp
  src/imgui/imgui_tables.cpp
  src/imgui/imgui_widgets.cpp
  src/imgui/imgui_demo.cpp
  src/imgui/imgui_impl_win32.cpp
  src/imgui/imgui_impl_dx10.cpp
)

target_link_libraries(${PROJECT_NAME}
  d3d10
  d3dcompiler
  dwmapi
  pthread
  ${CMAKE_CURRENT_SOURCE_DIR}/libusb-compat/libusb/.libs/libusb.a
  ${CMAKE_CURRENT_SOURCE_DIR}/libusb/libusb/.libs/libusb-1.0.a
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "-static -lgcc -lstdc++ -lwinpthread"
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS ""
  COMPILE_FLAGS ""
)
