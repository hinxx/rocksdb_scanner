#include "storage/rocksdb_schema_entry.hpp"
#include "storage/rocksdb_table_entry.hpp"
#include "storage/rocksdb_transaction.hpp"
#include "duckdb/catalog/dependency_list.hpp"
#include "duckdb/parser/parsed_data/create_table_info.hpp"
#include "duckdb/parser/parsed_data/create_view_info.hpp"
#include "duckdb/parser/parsed_data/create_index_info.hpp"
#include "duckdb/planner/parsed_data/bound_create_table_info.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"
#include "duckdb/parser/constraints/list.hpp"
#include "duckdb/common/unordered_set.hpp"
#include "duckdb/parser/parsed_data/alter_info.hpp"
#include "duckdb/parser/parsed_data/alter_table_info.hpp"
#include "duckdb/parser/parsed_expression_iterator.hpp"

namespace duckdb {

SQLiteSchemaEntry::SQLiteSchemaEntry(Catalog *catalog) : SchemaCatalogEntry(catalog, DEFAULT_SCHEMA, true) {
}

SQLiteTransaction &GetSQLiteTransaction(CatalogTransaction transaction) {
	if (!transaction.transaction) {
		throw InternalException("No transaction!?");
	}
	return (SQLiteTransaction &)*transaction.transaction;
}

string GetCreateTableSQL(Catalog *catalog, SchemaCatalogEntry *schema, CreateTableInfo &info) {
	for (idx_t i = 0; i < info.columns.LogicalColumnCount(); i++) {
		auto &col = info.columns.GetColumnMutable(LogicalIndex(i));
		col.SetType(SQLiteUtils::ToSQLiteType(col.GetType()));
	}

	std::stringstream ss;
	ss << "CREATE TABLE ";
	if (info.on_conflict == OnCreateConflict::IGNORE_ON_CONFLICT) {
		ss << "IF NOT EXISTS ";
	}
	ss << KeywordHelper::WriteOptionallyQuoted(info.table);
	ss << TableCatalogEntry::ColumnsToSQL(info.columns, info.constraints);
	ss << ";";
	return ss.str();
}

void SQLiteSchemaEntry::TryDropEntry(ClientContext &context, CatalogType catalog_type, const string &name) {
	DropInfo info;
	info.type = catalog_type;
	info.name = name;
	info.cascade = false;
	info.if_exists = true;
	DropEntry(context, &info);
}

CatalogEntry *SQLiteSchemaEntry::CreateTable(CatalogTransaction transaction, BoundCreateTableInfo *info) {
	auto &sqlite_transaction = GetSQLiteTransaction(transaction);
	auto &base_info = info->Base();
	auto table_name = base_info.table;
	if (base_info.on_conflict == OnCreateConflict::REPLACE_ON_CONFLICT) {
		// CREATE OR REPLACE - drop any existing entries first (if any)
		TryDropEntry(transaction.GetContext(), CatalogType::TABLE_ENTRY, table_name);
	}

	sqlite_transaction.GetDB().Execute(GetCreateTableSQL(catalog, this, base_info));
	return GetEntry(transaction, CatalogType::TABLE_ENTRY, table_name);
}

CatalogEntry *SQLiteSchemaEntry::CreateFunction(CatalogTransaction transaction, CreateFunctionInfo *info) {
	throw BinderException("SQLite databases do not support creating functions");
}

void UnqualifyColumnReferences(ParsedExpression &expr) {
	if (expr.type == ExpressionType::COLUMN_REF) {
		auto &colref = (ColumnRefExpression &)expr;
		auto name = std::move(colref.column_names.back());
		colref.column_names = {std::move(name)};
		return;
	}
	ParsedExpressionIterator::EnumerateChildren(expr, UnqualifyColumnReferences);
}

string GetCreateIndexSQL(CreateIndexInfo &info, TableCatalogEntry &tbl) {
	string sql;
	sql = "CREATE";
	if (info.constraint_type == IndexConstraintType::UNIQUE) {
		sql += " UNIQUE";
	}
	sql += " INDEX ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.index_name);
	sql += " ON ";
	sql += KeywordHelper::WriteOptionallyQuoted(tbl.name);
	sql += "(";
	for (idx_t i = 0; i < info.parsed_expressions.size(); i++) {
		if (i > 0) {
			sql += ", ";
		}
		UnqualifyColumnReferences(*info.parsed_expressions[i]);
		sql += info.parsed_expressions[i]->ToString();
	}
	sql += ")";
	return sql;
}

CatalogEntry *SQLiteSchemaEntry::CreateIndex(ClientContext &context, CreateIndexInfo *info, TableCatalogEntry *table) {
	auto &sqlite_transaction = SQLiteTransaction::Get(context, *table->catalog);
	sqlite_transaction.GetDB().Execute(GetCreateIndexSQL(*info, *table));
	return nullptr;
}

string GetCreateViewSQL(CreateViewInfo &info) {
	string sql;
	sql = "CREATE VIEW ";
	if (info.on_conflict == OnCreateConflict::IGNORE_ON_CONFLICT) {
		sql += "IF NOT EXISTS ";
	}
	sql += KeywordHelper::WriteOptionallyQuoted(info.view_name);
	sql += " ";
	if (!info.aliases.empty()) {
		sql += "(";
		for (idx_t i = 0; i < info.aliases.size(); i++) {
			if (i > 0) {
				sql += ", ";
			}
			auto &alias = info.aliases[i];
			sql += KeywordHelper::WriteOptionallyQuoted(alias);
		}
		sql += ") ";
	}
	sql += "AS ";
	sql += info.query->ToString();
	return sql;
}

CatalogEntry *SQLiteSchemaEntry::CreateView(CatalogTransaction transaction, CreateViewInfo *info) {
	if (info->sql.empty()) {
		throw BinderException("Cannot create view in SQLite that originated from an empty SQL statement");
	}
	if (info->on_conflict == OnCreateConflict::REPLACE_ON_CONFLICT) {
		// CREATE OR REPLACE - drop any existing entries first (if any)
		TryDropEntry(transaction.GetContext(), CatalogType::VIEW_ENTRY, info->view_name);
	}
	auto &sqlite_transaction = GetSQLiteTransaction(transaction);
	sqlite_transaction.GetDB().Execute(GetCreateViewSQL(*info));
	return GetEntry(transaction, CatalogType::VIEW_ENTRY, info->view_name);
}

CatalogEntry *SQLiteSchemaEntry::CreateSequence(CatalogTransaction transaction, CreateSequenceInfo *info) {
	throw BinderException("SQLite databases do not support creating sequences");
}

CatalogEntry *SQLiteSchemaEntry::CreateTableFunction(CatalogTransaction transaction, CreateTableFunctionInfo *info) {
	throw BinderException("SQLite databases do not support creating table functions");
}

CatalogEntry *SQLiteSchemaEntry::CreateCopyFunction(CatalogTransaction transaction, CreateCopyFunctionInfo *info) {
	throw BinderException("SQLite databases do not support creating copy functions");
}

CatalogEntry *SQLiteSchemaEntry::CreatePragmaFunction(CatalogTransaction transaction, CreatePragmaFunctionInfo *info) {
	throw BinderException("SQLite databases do not support creating pragma functions");
}

CatalogEntry *SQLiteSchemaEntry::CreateCollation(CatalogTransaction transaction, CreateCollationInfo *info) {
	throw BinderException("SQLite databases do not support creating collations");
}

CatalogEntry *SQLiteSchemaEntry::CreateType(CatalogTransaction transaction, CreateTypeInfo *info) {
	throw BinderException("SQLite databases do not support creating types");
}

void SQLiteSchemaEntry::AlterTable(SQLiteTransaction &sqlite_transaction, RenameTableInfo &info) {
	string sql = "ALTER TABLE ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.name);
	sql += " RENAME TO ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.new_table_name);
	sqlite_transaction.GetDB().Execute(sql);
}

void SQLiteSchemaEntry::AlterTable(SQLiteTransaction &sqlite_transaction, RenameColumnInfo &info) {
	string sql = "ALTER TABLE ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.name);
	sql += " RENAME COLUMN  ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.old_name);
	sql += " TO ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.new_name);
	sqlite_transaction.GetDB().Execute(sql);
}

void SQLiteSchemaEntry::AlterTable(SQLiteTransaction &sqlite_transaction, AddColumnInfo &info) {
	if (info.if_column_not_exists) {
		if (sqlite_transaction.GetDB().ColumnExists(info.name, info.new_column.GetName())) {
			return;
		}
	}
	string sql = "ALTER TABLE ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.name);
	sql += " ADD COLUMN  ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.new_column.Name());
	sql += " ";
	sql += info.new_column.Type().ToString();
	sqlite_transaction.GetDB().Execute(sql);
}

void SQLiteSchemaEntry::AlterTable(SQLiteTransaction &sqlite_transaction, RemoveColumnInfo &info) {
	if (info.if_column_exists) {
		if (!sqlite_transaction.GetDB().ColumnExists(info.name, info.removed_column)) {
			return;
		}
	}
	string sql = "ALTER TABLE ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.name);
	sql += " DROP COLUMN  ";
	sql += KeywordHelper::WriteOptionallyQuoted(info.removed_column);
	sqlite_transaction.GetDB().Execute(sql);
}

void SQLiteSchemaEntry::Alter(ClientContext &context, AlterInfo *info) {
	if (info->type != AlterType::ALTER_TABLE) {
		throw BinderException("Only altering tables is supported for now");
	}
	auto &alter = (AlterTableInfo &)*info;
	auto &transaction = SQLiteTransaction::Get(context, *catalog);
	switch (alter.alter_table_type) {
	case AlterTableType::RENAME_TABLE:
		AlterTable(transaction, (RenameTableInfo &)alter);
		break;
	case AlterTableType::RENAME_COLUMN:
		AlterTable(transaction, (RenameColumnInfo &)alter);
		break;
	case AlterTableType::ADD_COLUMN:
		AlterTable(transaction, (AddColumnInfo &)alter);
		break;
	case AlterTableType::REMOVE_COLUMN:
		AlterTable(transaction, (RemoveColumnInfo &)alter);
		break;
	default:
		throw BinderException("Unsupported ALTER TABLE type - SQLite tables only support RENAME TABLE, RENAME COLUMN, "
		                      "ADD COLUMN and DROP COLUMN");
	}
	transaction.ClearTableEntry(info->name);
}

void SQLiteSchemaEntry::Scan(ClientContext &context, CatalogType type,
                             const std::function<void(CatalogEntry *)> &callback) {
	auto &transaction = SQLiteTransaction::Get(context, *catalog);
	vector<string> entries;
	switch (type) {
	case CatalogType::TABLE_ENTRY:
		entries = transaction.GetDB().GetTables();
		break;
	case CatalogType::VIEW_ENTRY:
		entries = transaction.GetDB().GetEntries("view");
		break;
	case CatalogType::INDEX_ENTRY:
		entries = transaction.GetDB().GetEntries("index");
		break;
	default:
		// no entries of this catalog type
		return;
	}
	for (auto &entry_name : entries) {
		callback(GetEntry(GetCatalogTransaction(context), type, entry_name));
	}
}
void SQLiteSchemaEntry::Scan(CatalogType type, const std::function<void(CatalogEntry *)> &callback) {
	throw InternalException("Scan");
}

void SQLiteSchemaEntry::DropEntry(ClientContext &context, DropInfo *info) {
	switch (info->type) {
	case CatalogType::TABLE_ENTRY:
	case CatalogType::VIEW_ENTRY:
	case CatalogType::INDEX_ENTRY:
		break;
	default:
		throw BinderException("SQLite databases do not support dropping entries of type \"%s\"",
		                      CatalogTypeToString(type));
	}
	auto table = GetEntry(GetCatalogTransaction(context), info->type, info->name);
	if (!table) {
		throw InternalException("Failed to drop entry \"%s\" - could not find entry", info->name);
	}
	auto &transaction = SQLiteTransaction::Get(context, *catalog);
	transaction.DropEntry(info->type, info->name, info->cascade);
}

CatalogEntry *SQLiteSchemaEntry::GetEntry(CatalogTransaction transaction, CatalogType type, const string &name) {
	auto &sqlite_transaction = GetSQLiteTransaction(transaction);
	switch (type) {
	case CatalogType::INDEX_ENTRY:
	case CatalogType::TABLE_ENTRY:
	case CatalogType::VIEW_ENTRY:
		return sqlite_transaction.GetCatalogEntry(name);
	default:
		return nullptr;
	}
}

} // namespace duckdb