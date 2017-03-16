if(BUILD_TESTING)
  find_package(Boost 1.46 REQUIRED COMPONENTS unit_test_framework)
endif()

add_custom_command(
	OUTPUT
		lexer.c
		lexer.h
	DEPENDS ${CMAKE_SOURCE_DIR}/src/core/lexer.l
	COMMAND ${FLEX_EXECUTABLE}
		--outfile=lexer.c
		--header-file=lexer.h
		-- ${CMAKE_SOURCE_DIR}/src/core/lexer.l
	COMMENT "[FLEX][src] Building lexer with Flex ${FLEX_VERSION}"
	VERBATIM
)

set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
	PROPERTIES
		COMPILE_DEFINITIONS "_POSIX_SOURCE"
		COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter -Wno-type-limits -Wno-sign-compare"
)

add_library(
	libdeepstream_core SHARED
	client.cpp
	error_handler.cpp
	event.cpp
	exception.cpp
	impl.cpp
	message.cpp
	message_builder.cpp
	message_proxy.cpp
	parser.cpp
	presence.cpp
	random.cpp
	websockets.cpp
	websockets/pseudo.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c")

set_target_properties(libdeepstream_core PROPERTIES OUTPUT_NAME deepstream-core)
target_include_directories(libdeepstream_core PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
install(TARGETS libdeepstream_core DESTINATION "lib")

if(BUILD_TESTING AND Boost_FOUND)
  add_subdirectory(test)
endif()
