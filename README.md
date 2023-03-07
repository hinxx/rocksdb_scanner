# DuckDB rocksdbscanner extension

Attempt to create RocksDB scanner similar to Sqlite scanner.

       * renamed the files from sqlite_xxx.yyy to rocksdb_xxx.yyy
       * left the sqlite code parts intact
       * compiled as release (debug does not load the extension!?)
       * loaded extension

hinxx@obzen ~/ess/duckdb/rocksdb_scanner $ ./build/release/duckdb -unsigned

D install 'build/release/extension/rocksdb_scanner/rocksdb_scanner.duckdb_extension';
D load 'build/release/extension/rocksdb_scanner/rocksdb_scanner.duckdb_extension';
D select * from duckdb_extensions();
...
│ rocksdb_scanner   │ true    │ true      │ /home/hinxx/.duckdb/extensions/ab9736bed0/linux_amd…  │ Adds support for reading RocksDB database files                      │ []                │
...

D .maxrows 10000
D select distinct on(function_name) function_name, function_type, return_type, parameters, parameter_types from duckdb_functions() order by function_name;
...
│ rocksdb_attach       │ table         │                      │ [col0, overwrite]    │ [VARCHAR, BOOLEAN]                                                                                    │
│ rocksdb_scan         │ table         │                      │ [col0, col1]         │ [VARCHAR, VARCHAR]                                                                                    │
...


       * added some debug printouts
...
D load 'build/release/extension/rocksdb_scanner/rocksdb_scanner.duckdb_extension';
>>>RocksdbScanFunction
>>>RocksdbAttachFunction

       * added alias 'rocksdb' -> 'rocksdb_scanner'

│ rocksdb_scanner   │ true    │ true      │ /home/hinxx/.duckdb/extensions/7e68545119/linux_amd…  │ Adds support for reading RocksDB database files                      │ [rocksdb]         │

       * can do ATTACH, but my code is not called ?!

D ATTACH 'sakila.db' (TYPE rocksdb);

       * can do call rocksdb_attach('<name>') and then my code is called! Currently not accessing the RocksDB at all in this call

D call rocksdb_attach('/tmp/rocksdb_simple_example/');
>>>AttachBind
>>>AttachFunction
┌─────────┐
│ Success │
│ boolean │
├─────────┤
│ 0 rows  │
└─────────┘





























# DuckDB sqlitescanner extension

The sqlitescanner extension allows DuckDB to directly read data from a SQLite database file. The data can be queried directly from the underlying SQLite tables, or read into DuckDB tables.

## Usage

To make a SQLite file accessible to DuckDB, use the `ATTACH` command, for example with the bundled `sakila.db` file:
```sql
CALL sqlite_attach('sakila.db');
```

The tables in the file are registered as views in DuckDB, you can list them with
```sql
PRAGMA show_tables;
```

Then you can query those views normally using SQL, e.g. using the example queries from sakila-examples.sql

```sql
SELECT cat.name category_name,
       Sum(Ifnull(pay.amount, 0)) revenue
FROM   category cat
       LEFT JOIN film_category flm_cat
              ON cat.category_id = flm_cat.category_id
       LEFT JOIN film fil
              ON flm_cat.film_id = fil.film_id
       LEFT JOIN inventory inv
              ON fil.film_id = inv.film_id
       LEFT JOIN rental ren
              ON inv.inventory_id = ren.inventory_id
       LEFT JOIN payment pay
              ON ren.rental_id = pay.rental_id
GROUP  BY cat.name
ORDER  BY revenue DESC
LIMIT  5;
```


Or run them all in both SQLite and DuckDB
```bash
./duckdb/build/release/duckdb < sakila-examples.sql
sqlite3 sakila.db < sakila-examples.sql
```

## Building & Loading the Extension

To build, type
```
make
```

To run, run the bundled `duckdb` shell:
```
 ./build/release/duckdb -unsigned
```

Then, load the SQLite extension like so:
```SQL
LOAD 'build/release/extension/sqlite_scanner/sqlite_scanner.duckdb_extension';
```

