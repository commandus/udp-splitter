SUBDIRS = .
ACLOCAL_AMFLAGS = -I m4
AM_CXXFLAGS=-std=c++11 -O0

commoncppflags = -Ithird_party
GEN = gen/waves.h

$(GEN): Makefile
	tools/mkwav2code.sh	

#
#	Exclude generated sources from distribution
#
nodist_human_detector_SOURCES = $(GEN)
BUILT_SOURCES = $(GEN)
CLEANFILES = $(GEN)

#
# Binaries
#
bin_PROGRAMS = udp-splitter

nobase_dist_include_HEADERS = \
	platform.h utilstring.h \
	third_party/argtable3/argtable3.h \
	errlist.h daemonize.h

commonlibs = -L/usr/local/lib 

udp_splitter_SOURCES = \
	udp-splitter.cpp errlist.cpp daemonize.cpp \
	utilstring.cpp \
	third_party/argtable3/argtable3.c

udp_splitter_LDADD = $(commonlibs)
udp_splitter_CPPFLAGS = $(commoncppflags)

#
# Configs, readme, CMake etc.
#
configdir = $(datadir)
dist_config_DATA = \
   autogen.sh CMakeLists.txt CODE_OF_CONDUCT.md CONTRIBUTING.md COPYING HISTORY LICENSE README.md TODO \
   third_party/argtable3/README

check_PROGRAMS =

TESTS = $(check_PROGRAMS)
