# Copyright 2017 deepstreamHub GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_path(POCO_INCLUDE_DIR NAMES Poco/Poco.h)
# POCO insists on appending 'd' to the library names in debug mode
find_library(POCO_LIBRARY NAMES PocoFoundation PocoFoundationd)
find_library(POCO_NET_LIBRARY NAMES PocoNet PocoNetd)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	POCO REQUIRED_VARS POCO_LIBRARY POCO_INCLUDE_DIR)

if(POCO_FOUND)
	set(POCO_LIBRARIES ${POCO_LIBRARY} ${POCO_NET_LIBRARY})
	set(POCO_INCLUDE_DIRS ${POCO_INCLUDE_DIR})
endif()

mark_as_advanced(POCO_INCLUDE_DIR POCO_LIBRARY)
