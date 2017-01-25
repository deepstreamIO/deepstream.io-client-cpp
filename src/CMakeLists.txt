find_package(Boost 1.46 REQUIRED)
find_package(FLEX 2.5 REQUIRED)
find_package(POCO REQUIRED)


add_custom_command(
	OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/lexer.c
		${CMAKE_BINARY_DIR}/include/lexer.h
	DEPENDS lexer.l
	COMMAND ${FLEX_EXECUTABLE}
		--outfile=${CMAKE_CURRENT_BINARY_DIR}/lexer.c
		--header-file=${CMAKE_BINARY_DIR}/include/lexer.h
		-- ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
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
	deepstream SHARED
	client.cpp
	error_handler.cpp
	message.cpp
	message_builder.cpp
	message_proxy.cpp
	parser.cpp
	random.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c")

target_include_directories(deepstream PUBLIC "${CMAKE_BINARY_DIR}/include")
target_include_directories(deepstream PUBLIC ${Boost_INCLUDE_DIRS})


add_executable(ds-client executable.cpp)
target_include_directories(ds-client PUBLIC ${POCO_INCLUDE_DIRS})
target_link_libraries(ds-client PUBLIC deepstream)
target_link_libraries(ds-client PUBLIC ${POCO_LIBRARIES})
