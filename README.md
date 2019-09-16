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
		["conninfo"]="host=127.0.0.1 user=somebody password=password1 dbname=testdb"
	)
  ];
  Log::add_filter(Conn::LOG, filter);
  }
```

This will write to a database named testdb into the table named conn. Note that
the table will be automatically be created by the PostgreSQL plugin, if it does
not yet exist. If a table with the specified name already exists, it is used.

Configuration Options
=====================

The PostgreSQL writer supports the following configuration options that can be
passed in $config:

- *conninfo*: connection string using parameter key words as defined in
  https://www.postgresql.org/docs/9.3/static/libpq-connect.html. Can be used
  to pass usernames, passwords, etc. hostname, port, and dbname are ignored if
  conninfo is specified.

  Example: host=127.0.0.1 user=johanna

- *continue_on_errors*: ignore insert errors and do not kill the database
  connection.
