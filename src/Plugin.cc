
#include "Plugin.h"
#include "PostgresWriter.h"

namespace plugin {
	namespace CCX_PostgreSQL {
		Plugin plugin;
	}
}

using namespace plugin::CCX_PostgreSQL;

plugin::Configuration Plugin::Configure() {
	AddComponent(
		new ::logging::Component(
			"PostgreSQL",
			::logging::writer::PostgreSQL::Instantiate
		)
	);

	plugin::Configuration config;
	config.name = "CCX::PostgreSQL";
	config.description = "PostgreSQL log writer";
	config.version.major = 1;
	config.version.minor = 0;

	return config;
}
