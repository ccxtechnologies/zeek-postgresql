// See the file "COPYING" in the main distribution directory for copyright.

#include <string>
#include <errno.h>
#include <vector>
#include <regex>

#include "zeek-config.h"

#include "NetVar.h"
#include "threading/SerialTypes.h"
#include "threading/formatters/JSON.h"

#include "PostgresWriter.h"
#include "postgresql.bif.h"

using namespace logging;
using namespace writer;
using threading::Value;
using threading::Field;

PostgreSQL::PostgreSQL(WriterFrontend* frontend) : WriterBackend(frontend) {

	formatter = new threading::formatter::JSON(this,
			threading::formatter::JSON::TS_EPOCH);
	ignore_errors = false;
}

PostgreSQL::~PostgreSQL() {
	if ( conn != 0 ) {
		PQfinish(conn);
	}
}

// preformat the insert string that we only need to create once during our lifetime
bool PostgreSQL::CreateInsert(int num_fields, const Field* const * fields) {

	string names = "INSERT INTO " + table + " ( ";
	string values("VALUES (");

	for ( int i = 0; i < num_fields; ++i ) {
		string fieldname = EscapeIdentifier(fields[i]->name);
		if ( fieldname.empty() )
			return false;

		if ( i != 0 )
			{
			values += ", ";
			names += ", ";
			}

		names += fieldname;
		values += "$" + std::to_string(i+1);
	}

	insert = names + ") " + values + ") ;";

	return true;
}

string PostgreSQL::LookupParam(const WriterInfo& info, const string name) const {
	map<const char*, const char*>::const_iterator it = info.config.find(name.c_str());

	if ( it == info.config.end() ) {
		return string();
	} else {
		return it->second;
	}
}

string PostgreSQL::EscapeIdentifier(const char* identifier) {
	char* escaped = PQescapeIdentifier(conn, identifier, strlen(identifier));

	if ( escaped == nullptr ) {
		Error(Fmt("Error while escaping identifier '%s': %s",
			  identifier, PQerrorMessage(conn)));
		return string();
	}

	string out = escaped;
	PQfreemem(escaped);

	return out;
}

bool PostgreSQL::DoInit(const WriterInfo& info, int num_fields,
		const Field* const * fields) {

	string conninfo = LookupParam(info, "conninfo");
	string table_format = LookupParam(info, "format");

	string errorhandling = LookupParam(info, "continue_on_errors");
	if ( !errorhandling.empty() && errorhandling == "T" ) {
		ignore_errors = true;
	}

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

	data_column = LookupParam(info, "datacolumn");

	string create = "CREATE TABLE IF NOT EXISTS " + table +
		" (\n" + table_format + ");";

	PGresult *res = PQexec(conn, create.c_str());
	if ( PQresultStatus(res) != PGRES_COMMAND_OK) {
		Error(Fmt("Create command failed: %s\n", PQerrorMessage(conn)));
		return false;
	}

	return CreateInsert(num_fields, fields);
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

	vector<std::tuple<bool, string, int>> params;
	vector<const char*> params_char;
	vector<int> params_length;

	params.push_back(desc);

	for ( auto &i: params ) {
		if ( std::get<0>(i) == false) {
			params_char.push_back(nullptr); // null pointer is accepted to signify NULL in parameters
		} else {
			params_char.push_back(std::get<1>(i).c_str());
		}

		params_length.push_back(std::get<2>(i));
	}

	assert( params_char.size() == num_fields );
	assert( params_length.size() == num_fields );

	PGresult *res = PQexecParams(conn, insert.c_str(), params_char.size(),
				     NULL, &params_char[0], &params_length[0],
				     NULL, 0);

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
