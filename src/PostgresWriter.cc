// See the file "COPYING" in the main distribution directory for copyright.

#include <string>
#include <errno.h>
#include <vector>
#include <regex>

#include "zeek/zeek-config.h"

#include "zeek/NetVar.h"
#include "zeek/threading/SerialTypes.h"
#include "zeek/threading/formatters/JSON.h"

#include "PostgresWriter.h"
#include "postgresql.bif.h"

using namespace logging;
using namespace writer;
using zeek::threading::Value;
using zeek::threading::Field;

PostgreSQL::PostgreSQL(zeek::logging::WriterFrontend* frontend) : WriterBackend(frontend) {
	formatter = std::unique_ptr<zeek::threading::formatter::JSON>(
					new zeek::threading::formatter::JSON(this,
			zeek::threading::formatter::JSON::TS_EPOCH));
}

PostgreSQL::~PostgreSQL() {
	if ( conn != 0 ) {
		PQfinish(conn);
	}
}

std::string PostgreSQL::LookupParam(const WriterInfo& info, const std::string name) const {
	std::map<const char*, const char*>::const_iterator it = info.config.find(name.c_str());

	if ( it == info.config.end() ) {
		return std::string();
	} else {
		return it->second;
	}
}

std::string PostgreSQL::EscapeIdentifier(const char* identifier) {
	char* escaped = PQescapeIdentifier(conn, identifier, strlen(identifier));

	if ( escaped == nullptr ) {
		Error(Fmt("Error while escaping identifier '%s': %s",
			  identifier, PQerrorMessage(conn)));
		return std::string();
	}

	std::string out = escaped;
	PQfreemem(escaped);

	return out;
}

bool PostgreSQL::DoInit(const WriterInfo& info, int num_fields,
		const Field* const * fields) {

	std::string conninfo = LookupParam(info, "conninfo");

	columns = LookupParam(info, "columns");
	values = LookupParam(info, "values");
	indexes = LookupParam(info, "indexes");
	schema = LookupParam(info, "schema");

	conn = PQconnectdb(conninfo.c_str());

	if ( PQstatus(conn) != CONNECTION_OK ) {
		Error(Fmt("Could not connect to pg (%s): %s",
			conninfo.c_str(), PQerrorMessage(conn)));
		return false;
	}

	table = EscapeIdentifier(info.path);
	if ( table.empty() ) {
		return false;
	}

	std::string create = "CREATE SCHEMA IF NOT EXISTS \"" + schema + "\";";

	PGresult *res = PQexec(conn, create.c_str());
	if ( PQresultStatus(res) != PGRES_COMMAND_OK) {
		Error(Fmt("Create schema command failed: %s\n",
			  PQerrorMessage(conn)));
		return false;
	}

	table = schema + "." + table;

	create = "CREATE TABLE IF NOT EXISTS " +
		table + " (\n" + columns + ");";

	res = PQexec(conn, create.c_str());
	if ( PQresultStatus(res) != PGRES_COMMAND_OK) {
		Error(Fmt("Create table command failed: %s\n",
			  PQerrorMessage(conn)));
		return false;
	}

	insert = "INSERT INTO " + table + " ( " + indexes + " ) " +
		"VALUES ( " + values + " );";

	return true;
}

bool PostgreSQL::DoFlush(double network_time) {
	return true;
}

bool PostgreSQL::DoFinish(double network_time) {
	return true;
}

bool PostgreSQL::DoHeartbeat(double network_time, double current_time) {
	return true;
}

bool PostgreSQL::DoWrite(int num_fields,
		const Field* const* fields, Value** vals) {

	desc.Clear();
	if ( !formatter->Describe(&desc, num_fields, fields, vals) ) {
		return false;
	}

	const char* bytes = (const char*)desc.Bytes();
	int len = desc.Len();

	PGresult *res = PQexecParams(conn, insert.c_str(), 1, NULL,
				     &bytes, &len, NULL, 0);

	if ( PQresultStatus(res) != PGRES_COMMAND_OK) {
		Error(Fmt("Command failed: %s\n", PQerrorMessage(conn)));

		if ( ! ignore_errors ) {
			PQclear(res);
			return false;
		}
	}

	PQclear(res);
	return true;
}

bool PostgreSQL::DoRotate(const char* rotated_path, double open,
		double close, bool terminating) {
	FinishedRotation();
	return true;
}

bool PostgreSQL::DoSetBuf(bool enabled) {
	return true;
}
