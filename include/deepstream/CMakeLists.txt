add_subdirectory(websockets)

configure_file(config.h.in config.h @ONLY)

install(
	FILES
		buffer.hpp
		client.hpp
		error_handler.hpp
		event.hpp
		exception.hpp
		message.hpp
		message_builder.hpp
		message_proxy.hpp
		parser.h
		parser.hpp
		presence.hpp
		random.hpp
		scope_guard.hpp
		time.hpp
		use.hpp
		websockets.hpp
		"${CMAKE_BINARY_DIR}/include/deepstream/config.h"
	DESTINATION
		"include/deepstream"
)
