#ifndef DUCKDB_BUILD_LOADABLE_EXTENSION
#define DUCKDB_BUILD_LOADABLE_EXTENSION
#endif
#include "duckdb.hpp"

#include "rocksdb_scanner.hpp"
#include "rocksdb_storage.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"

using namespace duckdb;

extern "C" {

DUCKDB_EXTENSION_API void rocksdb_scanner_init(duckdb::DatabaseInstance &db) {
	Connection con(db);
	con.BeginTransaction();
	auto &context = *con.context;
	auto &catalog = Catalog::GetSystemCatalog(context);

	SqliteScanFunction sqlite_fun;
	CreateTableFunctionInfo sqlite_info(sqlite_fun);
	catalog.CreateTableFunction(context, &sqlite_info);

	SqliteAttachFunction attach_func;

	CreateTableFunctionInfo attach_info(attach_func);
	catalog.CreateTableFunction(context, &attach_info);

	auto &config = DBConfig::GetConfig(db);
	config.AddExtensionOption("rocksdb_all_varchar", "Load all RocksDB columns as VARCHAR columns", LogicalType::BOOLEAN);

	config.storage_extensions["rocksdb_scanner"] = make_unique<SQLiteStorageExtension>();

	con.Commit();
}

DUCKDB_EXTENSION_API const char *rocksdb_scanner_version() {
	return DuckDB::LibraryVersion();
}

DUCKDB_EXTENSION_API void rocksdb_scanner_storage_init(DBConfig &config) {
	config.storage_extensions["rocksdb_scanner"] = make_unique<SQLiteStorageExtension>();
}
}
