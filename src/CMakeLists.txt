include_directories(include)

set(ROCKSDB "/data/data/Code/rocksdb")
#list(APPEND CMAKE_PREFIX_PATH ${ROCKSDB})
include_directories(${ROCKSDB}/include)

# add_subdirectory(storage)

add_library(
  rocksdb_ext_library OBJECT
  rocksdb_db.cpp rocksdb_extension.cpp rocksdb_scanner.cpp rocksdb_stmt.cpp
  # rocksdb_storage.cpp rocksdb_utils.cpp)
  rocksdb_utils.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:rocksdb_ext_library>
    PARENT_SCOPE)
