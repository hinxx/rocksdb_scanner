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

set(ROCKSDB "/data/data/Code/rocksdb")
#list(APPEND CMAKE_PREFIX_PATH ${ROCKSDB})
# include_directories(${ROCKSDB}/include)

set(PARAMETERS "-no-warnings")
build_loadable_extension(${TARGET_NAME} ${PARAMETERS} ${ALL_OBJECT_FILES}
                         sqlite/sqlite3.c)

# g++ -fno-rtti simple_example.cc -osimple_example ../librocksdb_debug.a -I../include -O0 -std=c++17 
# -lpthread -lrt -ldl -lz -lbz2 -llz4 -lzstd -lnuma -std=c++17  -faligned-new 
# -DHAVE_ALIGNED_NEW -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX  -DOS_LINUX -fno-builtin-memcmp -DROCKSDB_FALLOCATE_PRESENT -DZLIB -DBZIP2 -DLZ4 -DZSTD -DNUMA -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -DROCKSDB_AUXV_GETAUXVAL_PRESENT -march=native   -DHAVE_SSE42  -DHAVE_PCLMUL -DHAVE_UINT128_EXTENSION   -ldl -lpthread
add_definitions(-faligned-new -fno-builtin-memcmp -march=native)
add_definitions(-DHAVE_ALIGNED_NEW -DROCKSDB_PLATFORM_POSIX -DROCKSDB_LIB_IO_POSIX -DOS_LINUX -DROCKSDB_FALLOCATE_PRESENT -DZLIB -DBZIP2 -DLZ4 -DZSTD -DNUMA -DROCKSDB_MALLOC_USABLE_SIZE -DROCKSDB_PTHREAD_ADAPTIVE_MUTEX -DROCKSDB_BACKTRACE -DROCKSDB_RANGESYNC_PRESENT -DROCKSDB_SCHED_GETCPU_PRESENT -DROCKSDB_AUXV_GETAUXVAL_PRESENT -DHAVE_SSE42  -DHAVE_PCLMUL -DHAVE_UINT128_EXTENSION)

# example from postgresql_scanner 
# target_link_libraries(${TARGET_NAME}_loadable_extension ${OPENSSL_LIBRARIES})
target_link_libraries(${TARGET_NAME}_loadable_extension ${ROCKSDB}/librocksdb.so bz2 z lz4 zstd numa)
