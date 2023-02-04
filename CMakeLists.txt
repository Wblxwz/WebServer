cmake_minimum_required(VERSION 3.25)

project(MyWebServer)

include_directories(${CMAKE_SOURCE_DIR}/server)

link_directories(/lib64 /lib64/mysql)

add_compile_options(-Wall -std=c++11)

aux_source_directory(./server SRC)

add_executable(WebServer ${SRC})

target_link_libraries(WebServer pthread mysqlclient)
