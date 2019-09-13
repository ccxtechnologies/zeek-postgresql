#ifndef ZEEK_PLUGIN_ZEEK_POSTGRES
#define ZEEK_PLUGIN_ZEEK_POSTGRES

#include <plugin/Plugin.h>

namespace plugin {
	namespace CCX_PostgreSQL {

		class Plugin : public ::plugin::Plugin {
			protected:
				// Overridden from plugin::Plugin.
				virtual plugin::Configuration Configure();
		};

		extern Plugin plugin;
	}
}

#endif
