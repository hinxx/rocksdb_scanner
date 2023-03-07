#include "storage/rocksdb_transaction.hpp"
#include "storage/rocksdb_catalog.hpp"
#include "storage/rocksdb_index_entry.hpp"
#include "storage/rocksdb_schema_entry.hpp"
#include "storage/rocksdb_table_entry.hpp"
#include "duckdb/parser/parsed_data/create_table_info.hpp"
#include "duckdb/parser/parsed_data/create_view_info.hpp"
#include "duckdb/catalog/catalog_entry/index_catalog_entry.hpp"
#include "duckdb/catalog/catalog_entry/view_catalog_entry.hpp"

namespace duckdb {

SQLiteTransaction::SQLiteTransaction(SQLiteCatalog &sqlite_catalog, TransactionManager &manager, ClientContext &context)
    : Transaction(manager, context), sqlite_catalog(sqlite_catalog) {
	if (sqlite_catalog.InMemory()) {
		// in-memory database - get a reference to the in-memory connection
		db = sqlite_catalog.GetInMemoryDatabase();
	} else {
		// on-disk database - open a new database connection
		owned_db = SQLiteDB::Open(sqlite_catalog.path,
		                          sqlite_catalog.access_mode == AccessMode::READ_ONLY ? true : false, true);
		db = &owned_db;
	}
}

SQLiteTransaction::~SQLiteTransaction() {
	sqlite_catalog.ReleaseInMemoryDatabase();
}

void SQLiteTransaction::Start() {
	db->Execute("BEGIN TRANSACTION");
}
void SQLiteTransaction::Commit() {
	db->Execute("COMMIT");
}
void SQLiteTransaction::Rollback() {
	db->Execute("ROLLBACK");
}

SQLiteDB &SQLiteTransaction::GetDB() {
	return *db;
}

SQLiteTransaction &SQLiteTransaction::Get(ClientContext &context, Catalog &catalog) {
	return (SQLiteTransaction &)Transaction::Get(context, catalog);
}

CatalogEntry *SQLiteTransaction::GetCatalogEntry(const string &entry_name) {
	auto entry = catalog_entries.find(entry_name);
	if (entry != catalog_entries.end()) {
		return entry->second.get();
	}
	// catalog entry not found - look up table in main SQLite database
	auto type = db->GetEntryType(entry_name);
	if (type == CatalogType::INVALID) {
		// no table or view found
		return nullptr;
	}
	unique_ptr<CatalogEntry> result;
	switch (type) {
	case CatalogType::TABLE_ENTRY: {
		CreateTableInfo info(sqlite_catalog.GetMainSchema(), entry_name);
		// FIXME: all_varchar from config
		db->GetTableInfo(entry_name, info.columns, info.constraints, false);
		D_ASSERT(!info.columns.empty());

		result = make_unique<SQLiteTableEntry>(&sqlite_catalog, sqlite_catalog.GetMainSchema(), info);
		break;
	}
	case CatalogType::VIEW_ENTRY: {
		string sql;
		db->GetViewInfo(entry_name, sql);

		auto view_info = CreateViewInfo::FromCreateView(*context.lock(), sql);
		view_info->internal = false;
		result = make_unique<ViewCatalogEntry>(&sqlite_catalog, sqlite_catalog.GetMainSchema(), view_info.get());
		break;
	}
	case CatalogType::INDEX_ENTRY: {
		CreateIndexInfo info;
		info.index_name = entry_name;

		string table_name;
		string sql;
		db->GetIndexInfo(entry_name, sql, table_name);

		auto index_entry = make_unique<SQLiteIndexEntry>(&sqlite_catalog, sqlite_catalog.GetMainSchema(), &info,
		                                                 std::move(table_name));
		index_entry->sql = std::move(sql);
		result = std::move(index_entry);
		break;
	}
	default:
		throw InternalException("Unrecognized catalog entry type");
	}
	auto result_ptr = result.get();
	catalog_entries[entry_name] = std::move(result);
	return result_ptr;
}

void SQLiteTransaction::ClearTableEntry(const string &table_name) {
	catalog_entries.erase(table_name);
}

string GetDropSQL(CatalogType type, const string &table_name, bool cascade) {
	string result;
	result = "DROP ";
	switch (type) {
	case CatalogType::TABLE_ENTRY:
		result += "TABLE ";
		break;
	case CatalogType::VIEW_ENTRY:
		result += "VIEW ";
		break;
	case CatalogType::INDEX_ENTRY:
		result += "INDEX ";
		break;
	default:
		throw InternalException("Unsupported type for drop");
	}
	result += KeywordHelper::WriteOptionallyQuoted(table_name);
	return result;
}

void SQLiteTransaction::DropEntry(CatalogType type, const string &table_name, bool cascade) {
	catalog_entries.erase(table_name);
	db->Execute(GetDropSQL(type, table_name, cascade));
}

} // namespace duckdb
