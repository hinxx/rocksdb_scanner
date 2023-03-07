#include "storage/rocksdb_catalog.hpp"
#include "storage/rocksdb_table_entry.hpp"
#include "storage/rocksdb_transaction.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "rocksdb_scanner.hpp"
#include "duckdb/storage/table_storage_info.hpp"

namespace duckdb {

SQLiteTableEntry::SQLiteTableEntry(Catalog *catalog, SchemaCatalogEntry *schema, CreateTableInfo &info)
    : TableCatalogEntry(catalog, schema, info) {
}

unique_ptr<BaseStatistics> SQLiteTableEntry::GetStatistics(ClientContext &context, column_t column_id) {
	return nullptr;
}

TableFunction SQLiteTableEntry::GetScanFunction(ClientContext &context, unique_ptr<FunctionData> &bind_data) {
	auto result = make_unique<RocksdbBindData>();
	for (auto &col : columns.Logical()) {
		result->names.push_back(col.GetName());
		result->types.push_back(col.GetType());
	}
	auto sqlite_catalog = (SQLiteCatalog *)catalog;
	result->file_name = sqlite_catalog->path;
	result->table_name = name;

	auto &transaction = (SQLiteTransaction &)Transaction::Get(context, *catalog);
	auto &db = transaction.GetDB();

	if (!db.GetMaxRowId(name, result->max_rowid)) {
		result->max_rowid = idx_t(-1);
		result->rows_per_group = idx_t(-1);
	}
	if (!transaction.IsReadOnly() || sqlite_catalog->InMemory()) {
		// for in-memory databases or if we have transaction-local changes we can only do a single-threaded scan
		// set up the transaction's connection object as the global db
		result->global_db = &db;
		result->rows_per_group = idx_t(-1);
	}

	bind_data = std::move(result);
	return RocksdbScanFunction();
}

TableStorageInfo SQLiteTableEntry::GetStorageInfo(ClientContext &context) {
	auto &transaction = (SQLiteTransaction &)Transaction::Get(context, *catalog);
	auto &db = transaction.GetDB();
	TableStorageInfo result;
	if (!db.GetMaxRowId(name, result.cardinality)) {
		// probably
		result.cardinality = 10000;
	}
	return result;
}

} // namespace duckdb
