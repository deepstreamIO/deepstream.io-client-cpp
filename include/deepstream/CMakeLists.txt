configure_file(config.h.in config.h @ONLY)

install(
	FILES "${CMAKE_BINARY_DIR}/include/deepstream/config.h"
	DESTINATION "include/deepstream"
)
