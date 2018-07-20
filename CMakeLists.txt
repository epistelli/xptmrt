cmake_minimum_required(VERSION 2.8)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(CMAKE_CL_64)
    add_definitions(-D_AMD64_)
    link_directories("${CMAKE_CURRENT_LIST_DIR}/lib/x64")
else()
    add_definitions(-D_X86_)
    link_directories("${CMAKE_CURRENT_LIST_DIR}/lib/x86")
endif()

add_library(xernel32 SHARED kernel32.cpp kernel32_impl.cpp kernel32.def)
target_link_libraries(xernel32 ntdll)
set_target_properties(xernel32 PROPERTIES LINK_FLAGS "/ignore:4197")

add_executable(test test.cpp)
target_link_libraries(test ntdll)