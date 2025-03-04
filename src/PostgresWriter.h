// See the file "COPYING" in the main distribution directory for copyright.
//
// Log writer for POSTGRES logs.

#ifndef LOGGING_WRITER_POSTGRES_H
#define LOGGING_WRITER_POSTGRES_H

#include <string>
#include <memory> // for unique_ptr

#include "zeek/Desc.h"
#include "zeek/logging/WriterBackend.h"
#include "zeek/threading/formatters/JSON.h"

#include "libpq-fe.h"

namespace logging {
	namespace writer {
		class PostgreSQL : public zeek::logging::WriterBackend {
		public:
			PostgreSQL(zeek::logging::WriterFrontend* frontend);
			~PostgreSQL();

			// prohibit copying and moving
			PostgreSQL(const PostgreSQL&) = delete;
			PostgreSQL& operator=(const PostgreSQL&) = delete;
			PostgreSQL(PostgreSQL&&) = delete;

			static zeek::logging::WriterBackend* Instantiate(zeek::logging::WriterFrontend* frontend) {
				return new PostgreSQL(frontend);
			}

		protected:
			bool DoInit(const WriterInfo& info, int num_fields,
				const zeek::threading::Field* const* fields) override;
			bool DoWrite(int num_fields, const zeek::threading::Field*
				const* fields, zeek::threading::Value** vals) override;
			bool DoSetBuf(bool enabled) override;
			bool DoRotate(const char* rotated_path, double open,
				double close, bool terminating) override;
			bool DoFlush(double network_time) override;
			bool DoFinish(double network_time) override;
			bool DoHeartbeat(double network_time,
				double current_time) override;

		private:
			std::string LookupParam(const WriterInfo& info,
				const std::string name) const;

			// note - EscapeIdentifier is replicated in reader
			std::string EscapeIdentifier(const char* identifier);
			std::tuple<bool, std::string, int> CreateParams(const zeek::threading::Value* val);
			std::string GetTableType(int, int);
			bool CreateInsert(int num_fields, const zeek::threading::Field* const* fields);

			PGconn *conn;

			std::string table;
			std::string insert;

			std::string columns;
			std::string values;
			std::string indexes;
			std::string schema;

			bool ignore_errors;

			std::unique_ptr<zeek::threading::formatter::JSON> formatter;
			zeek::ODesc desc;
		};

	}
}

#endif /* LOGGING_WRITER_POSTGRES_H */
