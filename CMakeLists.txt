cmake_minimum_required(VERSION 2.8)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# build statically
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(COMPFLAGS
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(COMPFLAG ${COMPFLAGS})
      string(REPLACE "/MD" "/MT" ${COMPFLAG} "${${COMPFLAG}}")
    endforeach()
endif()


if (CMAKE_CL_64)
    add_definitions(-D_AMD64_)
    link_directories("${CMAKE_CURRENT_LIST_DIR}/lib/x64")
else ()
    add_definitions(-D_X86_)
    link_directories("${CMAKE_CURRENT_LIST_DIR}/lib/x86")
endif ()

set(RT_MODULES "kernel32")
foreach (MOD ${RT_MODULES})
    string(REGEX REPLACE  "[a-z]([^ ]*)" "x\\1"  XMOD ${MOD})
    # build the module
    add_library(${XMOD} SHARED "${MOD}.cpp" "${MOD}_impl.cpp" "${MOD}.def")
    target_link_libraries(${XMOD} ntdll)
    set_target_properties(${XMOD} PROPERTIES LINK_FLAGS "/ignore:4197")
endforeach ()

add_executable(test test.cpp)
target_link_libraries(test ntdll)