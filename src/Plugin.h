#ifndef ZEEK_PLUGIN_ZEEK_POSTGRES
#define ZEEK_PLUGIN_ZEEK_POSTGRES

#include <zeek/plugin/Plugin.h>

namespace plugin {
	namespace CCX_PostgreSQL {

		class Plugin : public zeek::plugin::Plugin {
			protected:
				// Overridden from plugin::Plugin.
				virtual zeek::plugin::Configuration Configure();
		};

		extern Plugin plugin;
	}
}

#endif
