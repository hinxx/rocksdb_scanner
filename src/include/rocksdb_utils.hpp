//===----------------------------------------------------------------------===//
//                         DuckDB
//
// rocksdb_utils.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"
// REMOVE ME!
#include "sqlite3.h"

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"

namespace duckdb {

class SQLiteUtils {
public:
	static void Check(int rc, sqlite3 *db);
	static string TypeToString(int sqlite_type);
	static LogicalType TypeToLogicalType(const string &sqlite_type);
	static string SanitizeString(const string &table_name);
	static string SanitizeIdentifier(const string &table_name);
	static LogicalType ToSQLiteType(const LogicalType &input);
	string ToSQLiteTypeAlias(const LogicalType &input);
};

} // namespace duckdb
