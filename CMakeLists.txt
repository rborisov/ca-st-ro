cmake_minimum_required(VERSION 2.6)

project (player)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "2")

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(LibMPDClient REQUIRED)
find_package(GTK2 REQUIRED gtk)
find_package(Sqlite3 REQUIRED)
find_package(CURL REQUIRED)

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/external
    ${GTK2_INCLUDE_DIRS} ${LIBMPDCLIENT_INCLUDE_DIR} ${SQLITE3_INCLUDE_DIRS}
    )

    link_directories(${GTK2_LIBRARY_DIRS} ${LIBMPDCLIENT_LIBRARY_DIRS} ${SQLITE3_LIBRARY_DIRS}
        ${LIBNOTIFY_LIBRARY_DIRS} )

include(CheckCSourceCompiles)

set(CMAKE_C_FLAGS "-std=gnu99 -Wall -g -rdynamic") 
set(CMAKE_C_FLAGS_DEBUG "-ggdb -pedantic")

get_target_property(MKDATA_EXE mkdata LOCATION)

set(SOURCES
    main.c
    mpd_utils.c
    gtk_utils.c
    sql.c
    db_utils.c
    utils.c
    memory_utils.c
    socket_utils.c 
    external/json.c
    web_utils.c
)

add_executable(player ${SOURCES})
target_link_libraries(player ${CMAKE_THREAD_LIBS_INIT} ${GTK2_LIBRARIES} 
    ${LIBMPDCLIENT_LIBRARIES} ${SQLITE3_LIBRARIES} -lm
    ${CURL_LIBRARIES})

install(TARGETS player DESTINATION bin)
