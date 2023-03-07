//===----------------------------------------------------------------------===//
//                         DuckDB
//
// rocksdb_storage.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/storage/storage_extension.hpp"

namespace duckdb {

class RocksdbStorageExtension : public StorageExtension {
public:
	RocksdbStorageExtension();
};

} // namespace duckdb
