# CMakeList.txt : CMake project for Server, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Boost_Server")

# Add source to this project's executable.
find_package(Boost REQUIRED)
add_definitions(-D_WIN32_WINNT=0x0601)
add_executable (Boost_Server "src/Server.cpp" "headers/Server.h")

if(Boost_FOUND)
   target_include_directories(Boost_Server PRIVATE ${Boost_INCLUDE_DIRS})
   target_link_libraries(Boost_Server PRIVATE ${Boost_LIBRARIES})
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Boost_Server PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
