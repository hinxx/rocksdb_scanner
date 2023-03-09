cmake_minimum_required(VERSION 2.8.12)
set(TARGET_NAME rocksdb_scanner)
project(${TARGET_NAME})
include_directories(sqlite)

add_subdirectory(src)

# list(APPEND CMAKE_PREFIX_PATH "/home/hinxx/ess/rocksdb/")

# find_library(ROCKSDB_LIB rocksdb)
# target_link_libraries(shell PRIVATE "${ROCKSDB_LIB}")
# find_library(ROCKSDB_INC rocksdb/db.h)
# target_include_directories(shell PRIVATE "${ROCKSDB_INC}/..")

set(PARAMETERS "-no-warnings")
build_loadable_extension(${TARGET_NAME} ${PARAMETERS} ${ALL_OBJECT_FILES}
                         sqlite/sqlite3.c)
