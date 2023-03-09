//===----------------------------------------------------------------------===//
//                         DuckDB
//
// rocksdb_scanner.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"

namespace duckdb {
// REMOVE ME!
class SQLiteDB;

class RocksDB;

struct RocksdbBindData : public TableFunctionData {
	string file_name;
	string table_name;

	vector<string> names;
	vector<LogicalType> types;

	idx_t max_rowid = 0;
	bool all_varchar = false;

	idx_t rows_per_group = 122880;
	// REMOVE ME!
	SQLiteDB *global_db;

	string rocksdb_path_name;
	RocksDB *rocksdb_global_db;
};

class RocksdbScanFunction : public TableFunction {
public:
	RocksdbScanFunction();
};

class RocksdbAttachFunction : public TableFunction {
public:
	RocksdbAttachFunction();
};

} // namespace duckdb
