
project(Plugin)

include(BroPlugin)

find_package(PostgreSQL)

if (PostgreSQL_FOUND)
    include_directories(BEFORE ${PostgreSQL_INCLUDE_DIRS})
    link_directories(${PostgreSQL_LIBRARY_DIRS})

    zeek_plugin_begin(CCX PostgreSQL)
    zeek_plugin_cc(src/PostgresWriter.cc)
    zeek_plugin_cc(src/Plugin.cc)
    zeek_plugin_bif(src/postgresql.bif)
    zeek_plugin_dist_files(README COPYING VERSION)
    zeek_plugin_link_library(${PostgreSQL_LIBRARIES})
    zeek_plugin_end()

    message(STATUS "PostgreSQL includes : ${PostgreSQL_INCLUDE_DIRS}")
	message(STATUS "PostgreSQL libraries : ${PostgreSQL_LIBRARY_DIRS}")
else()
    message(FATAL_ERROR "PostgreSQL not found.")
endif()
