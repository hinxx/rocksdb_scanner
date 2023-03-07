add_library(
  rocksdb_ext_storage OBJECT
  rocksdb_catalog.cpp
  rocksdb_delete.cpp
  rocksdb_index.cpp
  rocksdb_index_entry.cpp
  rocksdb_insert.cpp
  rocksdb_table_entry.cpp
  rocksdb_schema_entry.cpp
  rocksdb_transaction.cpp
  rocksdb_transaction_manager.cpp
  rocksdb_update.cpp)
set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:rocksdb_ext_storage>
    PARENT_SCOPE)
