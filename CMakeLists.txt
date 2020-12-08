cmake_minimum_required(VERSION 3.9)
project(udp-splitter)
project(udp-splitter VERSION 1.0)

INCLUDE_DIRECTORIES("third_party")

set(UDP_SPLITTER_SRC 
	udp-splitter.cpp errlist.cpp daemonize.cpp utilstring.cpp
	third_party/argtable3/argtable3.c
)
add_executable(udp-splitter ${UDP_SPLITTER_SRC})
# target_link_libraries(udp-splitter ev)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/udp-splitter DESTINATION cmake)
INSTALL(FILES README.md LICENSE HISTORY COPYING TODO CODE_OF_CONDUCT.md CONTRIBUTING.md DESTINATION doc)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${udp_splitter_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${udp_splitter_VERSION_MINOR}")
include(CPack)