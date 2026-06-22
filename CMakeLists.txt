cmake_minimum_required(VERSION 4.3)

project(Raycasting LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)	

set(SFML_BUILD_AUDIO False)

message("Fetching SFML...")
FetchContent_Declare(
	SFML
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 3.1.0
)
FetchContent_MakeAvailable(SFML)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
set(BUILD_SHARED_LIBS TRUE)

add_executable(Raycasting ./src/Main.cpp)

target_link_libraries(Raycasting PRIVATE sfml-graphics)

if(WIN32)
  if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET Raycasting POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:sfml-graphics>
          $<TARGET_FILE:sfml-window>
          $<TARGET_FILE:sfml-system>
          $<TARGET_FILE_DIR:Raycasting>)
  endif()
endif()