// See the file "COPYING" in the main distribution directory for copyright.
//
// Log writer for POSTGRES logs.

#ifndef LOGGING_WRITER_POSTGRES_H
#define LOGGING_WRITER_POSTGRES_H

#include <memory> // for unique_ptr

#include "logging/WriterBackend.h"
#include "threading/formatters/JSON.h"

#include "libpq-fe.h"

namespace logging {
	namespace writer {
		class PostgreSQL : public WriterBackend {
		public:
			PostgreSQL(WriterFrontend* frontend);
			~PostgreSQL();

			// prohibit copying and moving
			PostgreSQL(const PostgreSQL&) = delete;
			PostgreSQL& operator=(const PostgreSQL&) = delete;
			PostgreSQL(PostgreSQL&&) = delete;

			static WriterBackend* Instantiate(WriterFrontend* frontend) {
				return new PostgreSQL(frontend);
			}

		protected:
			bool DoInit(const WriterInfo& info, int num_fields,
				const threading::Field* const* fields) override;
			bool DoWrite(int num_fields, const threading::Field*
				const* fields, threading::Value** vals) override;
			bool DoSetBuf(bool enabled) override;
			bool DoRotate(const char* rotated_path, double open,
				double close, bool terminating) override;
			bool DoFlush(double network_time) override;
			bool DoFinish(double network_time) override;
			bool DoHeartbeat(double network_time,
				double current_time) override;

		private:
			string LookupParam(const WriterInfo& info,
				const string name) const;

			// note - EscapeIdentifier is replicated in reader
			string EscapeIdentifier(const char* identifier);
			std::tuple<bool, string, int> CreateParams(const threading::Value* val);
			string GetTableType(int, int);
			bool CreateInsert(int num_fields, const threading::Field* const* fields);

			PGconn *conn;

			string table;
			string insert;

			string columns;
			string values;
			string indexes;
			string schema;

			bool ignore_errors;

			threading::formatter::Formatter* formatter;
			ODesc desc;
		};

	}
}

#endif /* LOGGING_WRITER_POSTGRES_H */
