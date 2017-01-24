find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
	return()
endif()


configure_file(Doxyfile.in Doxyfile @ONLY)

add_custom_target(
	doc
	${DOXYGEN_EXECUTABLE} Doxyfile >/dev/null
	COMMENT Building Doxygen
)
