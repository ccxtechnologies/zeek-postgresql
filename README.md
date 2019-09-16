Zeek PostgreSQL JSON Databases Writer Plugin
============================================

This plugin is based on [zeek-postgres](https://github.com/0xxon/zeek-postgresql).

It makes some updates to the zeek support, removes the read feature, and changes
the data write format to JSON.

Installation
------------

After installing PostgreSQL, you can install the Bro PostgreSQL module
either using bro-pkg, or manually via the command-line.

To install the plugin using the Zeek Package Browser::

```console
# zkg install ccxtechnologies/zeek-postgresql-json
```

To install manually from the cloned repository::

```console
# ./configure && make && make install
```

Use bro -N to verify correct installation::

```console
# bro -N CCX::PostgreSQLJSON
CCX::PostgreSQL - PostgreSQL json log writer (dynamic, version 1.0)
```

Logging Data into PostgreSQL databases
--------------------------------------

The easiest way to add PostgreSQL logging is by adding a logging filter to an
already existing logging stream. This first example also sends the conn.log
to PostgreSQL:

```zeek
event zeek_init()
  {
  local filter: Log::Filter = [
  	$name="postgres",
	$path="conn",
	$writer=Log::WRITER_POSTGRESQL,
	$config=table(
		["conninfo"]="host=127.0.0.1 user=charles password=password dbname=testdb",
		["schema"]="test_zeek",
		["columns"]="_id serial PRIMARY KEY, _timestamp timestamp default current_time, _source varchar NOT NULL, data jsonb",
		["values"]="'test', $1",
		["indexes"]="_source, data",
	)
  ];
  Log::add_filter(Conn::LOG, filter);
  }
```

This will create a table conn in the schema test_zeek with the structure defined in columns.
Data will be written to the indexes columns (all other columns must have defaults) based on
values, the $1 indicates the column that the json data is written to, the other columns can
have static values.


Configuration Options
=====================

The PostgreSQL writer supports the following configuration options that can be
passed in $config:

- *conninfo*: connection string using parameter key words as defined in
  https://www.postgresql.org/docs/9.3/static/libpq-connect.html. Can be used
  to pass usernames, passwords, etc.

  Example: host=127.0.0.1 user=johanna

- *schema*: name of the scehma to insert data into

- *columns*: structure of the table

- *values*: values to write to each column, $1 indicates that the json data from Zeek is written

- *indexes*: columns that values are written into
