cmake_minimum_required(VERSION 3.22)
project(lab3 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_COMPILER "gcc")

set(SERVER server_)
set(CLIENT client_)

set(LIB_DB lab1)
set(LIB_DB_DIR lab1)
set(LIB_PARSER lab2)
set(LIB_PARSER_DIR lab2)

set(THRIFT_LIB thrift_c_glib)

add_library(${LIB_DB} STATIC
        ${LIB_DB_DIR}/queries.c
        ${LIB_DB_DIR}/queries.h
        ${LIB_DB_DIR}/database.c
        ${LIB_DB_DIR}/database.h
        ${LIB_DB_DIR}/table.c
        ${LIB_DB_DIR}/table.h
        ${LIB_DB_DIR}/page.c
        ${LIB_DB_DIR}/page.h
        )

target_include_directories(${LIB_DB} PUBLIC ${GLIB_INCLUDE_DIRS})


set(LIB_PARSER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(LEXER_OUT "${LIB_PARSER_OUT_DIR}/lex.yy.c")
set(PARSER_OUT "${LIB_PARSER_OUT_DIR}/parserLINQ.tab.c")

flex_target(LEXER "${LIB_PARSER_DIR}/lexerLINQ.l" ${LEXER_OUT} DEFINES_FILE "${LIB_PARSER_OUT_DIR}/lexer.h")
bison_target(PARSER "${LIB_PARSER_DIR}/parserLINQ.y" "${PARSER_OUT}" DEFINES_FILE "${LIB_PARSER_OUT_DIR}/parserLINQ.tab.h")
add_flex_bison_dependency(LEXER PARSER)

add_library(${LIB_PARSER} STATIC
        ${LIB_PARSER_DIR}/linq.c
        ${LIB_PARSER_DIR}/linq.h
        ${LIB_PARSER_DIR}/data.c
        ${LIB_PARSER_DIR}/data.h
        ${LIB_PARSER_DIR}/write.c
        ${LIB_PARSER_DIR}/write.h
        ${LEXER_OUT}
        ${PARSER_OUT}
        )
target_include_directories(${LIB_PARSER} PRIVATE ${LIB_PARSER_DIR} ${LIB_PARSER_OUT_DIR})


target_link_libraries(${LIB_DB})
find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0)

add_executable(${SERVER}
        server.c
        gen-c_glib/nt_structs_types.c
        gen-c_glib/nt_structs_types.h
        gen-c_glib/send_statement_service.c
        gen-c_glib/send_statement_service.h
        request_processor.h
        request_processor.c
        )
target_include_directories(${SERVER} PUBLIC ${GLIB_INCLUDE_DIRS})
target_link_libraries(${SERVER} PUBLIC ${LIB_DB} ${THRIFT_LIB} ${GLIB_LDFLAGS})

add_executable(${CLIENT}
        client.c
        gen-c_glib/nt_structs_types.c
        gen-c_glib/nt_structs_types.h
        gen-c_glib/send_statement_service.c
        gen-c_glib/send_statement_service.h
        serializer.c
        serializer.h
        )

target_include_directories(${CLIENT} PUBLIC ${GLIB_INCLUDE_DIRS})
target_link_libraries(${CLIENT} PUBLIC ${LIB_PARSER} ${THRIFT_LIB} ${GLIB_LDFLAGS})
