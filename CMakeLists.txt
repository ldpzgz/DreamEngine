# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.14)

include(FetchContent)
#FetchContent_Declare(glad
#        URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.2.zip)
#FetchContent_MakeAvailable(glad)

FetchContent_Declare(glm
        URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip
        )
FetchContent_MakeAvailable(glm)

#GLFW additions
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw URL https://github.com/glfw/glfw/archive/refs/tags/3.3.6.zip)
FetchContent_MakeAvailable(glfw)

#include(FetchContent) # once in the project to include the module
#FetchContent_Declare(googletest
#                     GIT_REPOSITORY https://github.com/google/googletest.git
#                     GIT_TAG        703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0)
#FetchContent_MakeAvailable(googletest)

#individual files：
#FetchContent_Declare(doctest URL https://raw.githubusercontent.com/doctest/doctest/v2.4.9/doctest/doctest.h)
#FetchContent_MakeAvailable(doctest)

#zipped folders：this is fast
#FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
#FetchContent_MakeAvailable(lexy)

#set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
#add_subdirectory(core)
#add_subdirectory(app)
#${PROJECT_NAME}
# 项目信息
project (DreamEngine)

set(CMAKE_CXX_STANDARD 17)
set(GLAD_DIR C:/myproject/opengles/glad)

include_directories(${glm_SOURCE_DIR})
include_directories(${GLAD_DIR}/include)
include_directories(C:/myproject/opengles/hello/rapidxml)
include_directories(${glfw_SOURCE_DIR}/include)
#link_directories(C:/myproject/opengles/glfw-3.3.6.bin.WIN64/lib-vc2022)
#----------------------------dreamEngine static lib--------------------
aux_source_directory(${PROJECT_SOURCE_DIR}/core de_core)
aux_source_directory(${PROJECT_SOURCE_DIR}/ui de_ui)
aux_source_directory(${PROJECT_SOURCE_DIR}/animation de_animation)

add_library(dreamEngine STATIC 
    ${de_core}
	${de_ui}
	${de_animation}
	${GLAD_DIR}/src/gl.c
)

target_include_directories(dreamEngine PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(dreamEngine
    PRIVATE
	glfw
)
#----------------------------dreamEngine static lib--------------------end

#----------------------------main test exectable--------------------
aux_source_directory(${PROJECT_SOURCE_DIR}/app testAppSource)

add_executable(deTest 
    ${testAppSource}
)

target_link_libraries(deTest
    PRIVATE 
    dreamEngine
)
target_include_directories(deTest PRIVATE 
	${PROJECT_SOURCE_DIR}
)
#----------------------------main test exectable--------------------end
#first cmake .. -DCMAKE_INSTALL_PREFIX=/install/location
#the "DESTINATION" option in the following command means ${CMAKE_INSTALL_PREFIX}
#the following command will copy the deTest target file to "${CMAKE_INSTALL_PREFIX}/bin"
#when you run "make install"
#other install commands:
#install (TARGETS cmake_examples_inst LIBRARY DESTINATION lib) 
#install (TARGETS cmake_examples_inst
#    LIBRARY DESTINATION lib
#    RUNTIME DESTINATION bin)  this works on windows for copy ".lib and .dll" files
#install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
#install (FILES cmake-examples.conf DESTINATION etc)
install (TARGETS deTest DESTINATION bin)