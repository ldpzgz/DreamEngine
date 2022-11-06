# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.6)

#set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
#add_subdirectory(core)
#add_subdirectory(app)
#${PROJECT_NAME}
# 项目信息
project (DreamEngine)

set(CMAKE_CXX_STANDARD 17)

include_directories(C:/myproject/opengles/glm)
include_directories(C:/myproject/opengles/glad/include)
include_directories(C:/myproject/opengles/hello/rapidxml)
include_directories(C:/myproject/opengles/glfw-3.3.6.bin.WIN64/include)
link_directories(C:/myproject/opengles/glfw-3.3.6.bin.WIN64/lib-vc2022)
#----------------------------dreamEngine static lib--------------------
aux_source_directory(${PROJECT_SOURCE_DIR}/core de_core)
aux_source_directory(${PROJECT_SOURCE_DIR}/ui de_ui)
aux_source_directory(${PROJECT_SOURCE_DIR}/animation de_animation)

add_library(dreamEngine STATIC 
    ${de_core}
	${de_ui}
	${de_animation}
)

target_include_directories(dreamEngine PRIVATE ${PROJECT_SOURCE_DIR})
#----------------------------dreamEngine static lib--------------------end

#----------------------------main test exectable--------------------
aux_source_directory(${PROJECT_SOURCE_DIR}/app testAppSource)

add_executable(test 
    ${testAppSource}
)

target_link_libraries(test
    PRIVATE 
    dreamEngine
	glfw3dll.lib
)
target_include_directories(test PRIVATE 
	${PROJECT_SOURCE_DIR}
)
#----------------------------main test exectable--------------------end