cmake_minimum_required(VERSION 3.21)
project(OpenGL_Study LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

find_package(OpenGL REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glfw3 CONFIG QUIET)

if (glfw3_FOUND)
    target_link_libraries(main PRIVATE glfw)
else()
    set(LOCAL_GLFW_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/libglfw3.a")

    if (NOT EXISTS "${LOCAL_GLFW_LIBRARY}")
        message(FATAL_ERROR "GLFW was not found. Install it with vcpkg or provide lib/libglfw3.a.")
    endif()

    add_library(glfw_local STATIC IMPORTED)
    set_target_properties(glfw_local PROPERTIES
        IMPORTED_LOCATION "${LOCAL_GLFW_LIBRARY}"
    )

    target_link_libraries(main PRIVATE glfw_local)
endif()

target_link_libraries(main PRIVATE
    OpenGL::GL
    gdi32
    assimp::assimp
)
