#include "storage/rocksdb_catalog.hpp"
#include "storage/rocksdb_index.hpp"
#include "duckdb/parser/statement/create_statement.hpp"
#include "duckdb/planner/operator/logical_extension_operator.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"

namespace duckdb {

SQLiteCreateIndex::SQLiteCreateIndex(unique_ptr<CreateIndexInfo> info, TableCatalogEntry &table)
    : PhysicalOperator(PhysicalOperatorType::EXTENSION, {LogicalType::BIGINT}, 1), info(std::move(info)), table(table) {
}

//===--------------------------------------------------------------------===//
// Source
//===--------------------------------------------------------------------===//
class SQLiteIndexSourceState : public GlobalSourceState {
public:
	SQLiteIndexSourceState() : finished(false) {
	}

	bool finished;
};

unique_ptr<GlobalSourceState> SQLiteCreateIndex::GetGlobalSourceState(ClientContext &context) const {
	return make_unique<SQLiteIndexSourceState>();
}

void SQLiteCreateIndex::GetData(ExecutionContext &context, DataChunk &chunk, GlobalSourceState &gstate,
                                LocalSourceState &lstate) const {
	auto &state = (SQLiteIndexSourceState &)gstate;
	if (state.finished) {
		return;
	}
	auto &catalog = *table.catalog;
	auto schema = catalog.GetSchema(context.client, info->schema);
	schema->CreateIndex(context.client, info.get(), &table);
	state.finished = true;
}

//===--------------------------------------------------------------------===//
// Logical Operator
//===--------------------------------------------------------------------===//
class LogicalSQLiteCreateIndex : public LogicalExtensionOperator {
public:
	LogicalSQLiteCreateIndex(unique_ptr<CreateIndexInfo> info_p, TableCatalogEntry &table)
	    : info(std::move(info_p)), table(table) {
	}

	unique_ptr<CreateIndexInfo> info;
	TableCatalogEntry &table;

	unique_ptr<PhysicalOperator> CreatePlan(ClientContext &context, PhysicalPlanGenerator &generator) override {
		return make_unique<SQLiteCreateIndex>(std::move(info), table);
	}

	void Serialize(FieldWriter &writer) const override {
		throw InternalException("Cannot serialize SQLite Create index");
	}

	void ResolveTypes() override {
		types = {LogicalType::BIGINT};
	}
};

unique_ptr<LogicalOperator> SQLiteCatalog::BindCreateIndex(Binder &binder, CreateStatement &stmt,
                                                           TableCatalogEntry &table, unique_ptr<LogicalOperator> plan) {
	return make_unique<LogicalSQLiteCreateIndex>(unique_ptr_cast<CreateInfo, CreateIndexInfo>(std::move(stmt.info)),
	                                             table);
}

} // namespace duckdb
