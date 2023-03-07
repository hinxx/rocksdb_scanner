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
class SQLiteDB;

struct RocksdbBindData : public TableFunctionData {
	string file_name;
	string table_name;

	vector<string> names;
	vector<LogicalType> types;

	idx_t max_rowid = 0;
	bool all_varchar = false;

	idx_t rows_per_group = 122880;
	SQLiteDB *global_db;
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
