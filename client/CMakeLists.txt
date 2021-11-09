cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

project(Multiplayer-Game-Client)

add_library(glad-lib ./libs/glad.c) # compile glad separately to supress pedantic warnings from it
target_include_directories(glad-lib PUBLIC ./include/)

add_executable(multi-client ./src/main.cpp ./src/app.cpp
	./src/mesh.cpp
	./src/networking/client.cpp
	./src/stbimpl.c)
target_include_directories(multi-client PUBLIC ./include/)
if(MINGW)
	target_link_directories(multi-client PUBLIC ./build/ ./libs/mingw/)
	target_link_libraries(multi-client PUBLIC glfw3 opengl32 gdi32 glad-lib ws2_32 wsock32)
elseif(WIN32)
	target_link_directories(multi-client PUBLIC ./build/ ./libs/win/)
	target_link_libraries(multi-client PUBLIC glfw3 opengl32 gdi32 glad-lib ws2_32 wsock32)
elseif(UNIX)
	target_link_directories(multi-client PUBLIC ./build/ ./libs/linux/)
	target_link_libraries(multi-client PUBLIC GL dl pthread X11 glfw3 glad-lib)
endif()
if(MSVC)
	target_compile_options(multi-client PUBLIC /W4 /WX)
else()
	target_compile_options(multi-client PUBLIC -Wall -Wextra -Wpedantic)
endif()
add_custom_command(TARGET multi-client POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_SOURCE_DIR}/README.md
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/README.md)
