#include "duckdb.hpp"

#include "sqlite3.h"
#include "rocksdb_utils.hpp"
#include "rocksdb_storage.hpp"
#include "storage/rocksdb_catalog.hpp"
#include "storage/rocksdb_transaction_manager.hpp"
#include "duckdb/parser/parsed_data/attach_info.hpp"
#include "duckdb/transaction/transaction_manager.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"

namespace duckdb {

static unique_ptr<Catalog> SQLiteAttach(StorageExtensionInfo *storage_info, AttachedDatabase &db, const string &name,
                                        AttachInfo &info, AccessMode access_mode) {
	return make_unique<SQLiteCatalog>(db, info.path, access_mode);
}

static unique_ptr<TransactionManager> SQLiteCreateTransactionManager(StorageExtensionInfo *storage_info,
                                                                     AttachedDatabase &db, Catalog &catalog) {
	auto &sqlite_catalog = (SQLiteCatalog &)catalog;
	return make_unique<SQLiteTransactionManager>(db, sqlite_catalog);
}

RocksdbStorageExtension::RocksdbStorageExtension() {
	attach = SQLiteAttach;
	create_transaction_manager = SQLiteCreateTransactionManager;
}

} // namespace duckdb
