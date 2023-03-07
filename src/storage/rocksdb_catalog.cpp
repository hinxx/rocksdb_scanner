#include "storage/rocksdb_catalog.hpp"
#include "storage/rocksdb_schema_entry.hpp"
#include "storage/rocksdb_transaction.hpp"
#include "rocksdb_db.hpp"
#include "duckdb/storage/database_size.hpp"

namespace duckdb {

SQLiteCatalog::SQLiteCatalog(AttachedDatabase &db_p, const string &path, AccessMode access_mode)
    : Catalog(db_p), path(path), access_mode(access_mode), in_memory(path == ":memory:"), active_in_memory(false) {
	if (InMemory()) {
		in_memory_db = SQLiteDB::Open(path, false, true);
	}
}

SQLiteCatalog::~SQLiteCatalog() {
}

void SQLiteCatalog::Initialize(bool load_builtin) {
	main_schema = make_unique<SQLiteSchemaEntry>(this);
}

CatalogEntry *SQLiteCatalog::CreateSchema(CatalogTransaction transaction, CreateSchemaInfo *info) {
	throw BinderException("SQLite databases do not support creating new schemas");
}

void SQLiteCatalog::ScanSchemas(ClientContext &context, std::function<void(CatalogEntry *)> callback) {
	callback(main_schema.get());
}

SchemaCatalogEntry *SQLiteCatalog::GetSchema(CatalogTransaction transaction, const string &schema_name, bool if_exists,
                                             QueryErrorContext error_context) {
	if (schema_name == DEFAULT_SCHEMA || schema_name == INVALID_SCHEMA) {
		return main_schema.get();
	}
	if (if_exists) {
		return nullptr;
	}
	throw BinderException("SQLite databases only have a single schema - \"%s\"", DEFAULT_SCHEMA);
}

bool SQLiteCatalog::InMemory() {
	return in_memory;
}

string SQLiteCatalog::GetDBPath() {
	return path;
}

SQLiteDB *SQLiteCatalog::GetInMemoryDatabase() {
	if (!InMemory()) {
		throw InternalException("GetInMemoryDatabase() called on a non-in-memory database");
	}
	lock_guard<mutex> l(in_memory_lock);
	if (active_in_memory) {
		throw TransactionException("Only a single transaction can be active on an in-memory SQLite database at a time");
	}
	active_in_memory = true;
	return &in_memory_db;
}

void SQLiteCatalog::ReleaseInMemoryDatabase() {
	if (!InMemory()) {
		return;
	}
	lock_guard<mutex> l(in_memory_lock);
	if (!active_in_memory) {
		throw InternalException(
		    "ReleaseInMemoryDatabase called but there is no active transaction on an in-memory database");
	}
	active_in_memory = false;
}

void SQLiteCatalog::DropSchema(ClientContext &context, DropInfo *info) {
	throw BinderException("SQLite databases do not support dropping schemas");
}

DatabaseSize SQLiteCatalog::GetDatabaseSize(ClientContext &context) {
	DatabaseSize result;

	auto &transaction = SQLiteTransaction::Get(context, *this);
	auto &db = transaction.GetDB();
	result.total_blocks = db.RunPragma("page_count");
	result.block_size = db.RunPragma("page_size");
	result.free_blocks = db.RunPragma("freelist_count");
	result.used_blocks = result.total_blocks - result.free_blocks;
	result.bytes = result.total_blocks * result.block_size;
	result.wal_size = idx_t(-1);
	return result;
}

} // namespace duckdb
