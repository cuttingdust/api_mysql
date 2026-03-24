cmake_minimum_required(VERSION 3.20)
get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${CURRENT_DIR})

set(DPS_LIBRARYS
    MySQL::MySQL
    fmt::fmt
	utf8cpp::utf8cpp
)

cpp_execute(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE 
			-DTRINITY_MYSQL_FLAVOR="MySQL"
			-DTRINITY_REQUIRED_MYSQL_VERSION="8.0.34"
			-DTRINITY_REQUIRED_WINDOWS_BUILD=18362
			)

if(MSVC)
    #设为启动项

    #set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) 
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${OUT_RUN_PATH})
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "MFunction")

    #set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/GL")
    #set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/LTCG")
endif()