add_subdirectory(deepstream)

install(
	DIRECTORY ./
	DESTINATION "include"
	FILES_MATCHING REGEX ".+[.](h|hpp)"
)
