cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

project(Multiplayer-Game-Client)

add_executable(multi-client ./src/main.cpp)
target_include_directories(multi-client PUBLIC ./include/)
if(MINGW)
	target_link_directories(multi-client PUBLIC ./libs/mingw/)
	target_link_libraries(multi-client PUBLIC glfw3 opengl32 gdi32)
elseif(WIN32)
	target_link_directories(multi-client PUBLIC ./libs/win/)
	target_link_libraries(multi-client PUBLIC glfw3 opengl32 gdi32)
elseif(UNIX)
	target_link_directories(multi-client PUBLIC ./libs/linux/)
	target_link_libraries(multi-client PUBLIC GL dl pthread X11 glfw3)
endif()
