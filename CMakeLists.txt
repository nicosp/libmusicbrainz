cmake_minimum_required(VERSION 3.5)

PROJECT(libmusicbrainz5)
SET(PROJECT_VERSION 5.1.0)

# 1. If the library source code has changed at all since the last update, then increment revision.
# 2. If any interfaces have been added, removed, or changed since the last update, increment current, and set revision to 0.
# 3. If any interfaces have been added since the last public release, then increment age.
# 4. If any interfaces have been removed since the last public release, then set age to 0.
SET(musicbrainz5_SOVERSION_CURRENT  2)
SET(musicbrainz5_SOVERSION_REVISION 0)
SET(musicbrainz5_SOVERSION_AGE      0)

MATH(EXPR musicbrainz5_SOVERSION_MAJOR "${musicbrainz5_SOVERSION_CURRENT} - ${musicbrainz5_SOVERSION_AGE}")
MATH(EXPR musicbrainz5_SOVERSION_MINOR "${musicbrainz5_SOVERSION_AGE}")
MATH(EXPR musicbrainz5_SOVERSION_PATCH "${musicbrainz5_SOVERSION_REVISION}")

SET(musicbrainz5_VERSION ${musicbrainz5_SOVERSION_MAJOR}.${musicbrainz5_SOVERSION_MINOR}.${musicbrainz5_SOVERSION_PATCH})
SET(musicbrainz5_SOVERSION ${musicbrainz5_SOVERSION_MAJOR})

SET(musicbrainz5c_SOVERSION_CURRENT  2)
SET(musicbrainz5c_SOVERSION_REVISION 0)
SET(musicbrainz5c_SOVERSION_AGE      0)

MATH(EXPR musicbrainz5c_SOVERSION_MAJOR "${musicbrainz5c_SOVERSION_CURRENT} - ${musicbrainz5c_SOVERSION_AGE}")
MATH(EXPR musicbrainz5c_SOVERSION_MINOR "${musicbrainz5c_SOVERSION_AGE}")
MATH(EXPR musicbrainz5c_SOVERSION_PATCH "${musicbrainz5c_SOVERSION_REVISION}")

SET(musicbrainz5c_VERSION ${musicbrainz5c_SOVERSION_MAJOR}.${musicbrainz5c_SOVERSION_MINOR}.${musicbrainz5c_SOVERSION_PATCH})
SET(musicbrainz5c_SOVERSION ${musicbrainz5c_SOVERSION_MAJOR})

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
FIND_PACKAGE(CURL REQUIRED)
IF(NOT WIN32)
	FIND_PACKAGE(Threads REQUIRED)
ENDIF(NOT WIN32)
FIND_PACKAGE(LibXml2 REQUIRED)

OPTION(BUILD_SHARED_LIBS "Build shared libraries (static ones otherwise)" ON)

# With MSVC, programs linking the static libraries must not import the
# symbols from a DLL (see <musicbrainz5/export.h>)
SET(MB5_STATIC_CFLAGS "")
IF(NOT BUILD_SHARED_LIBS)
	ADD_DEFINITIONS(-DMB5_STATIC)
	SET(MB5_STATIC_CFLAGS " -DMB5_STATIC")
ENDIF(NOT BUILD_SHARED_LIBS)

# A static curl or libxml2 needs the libraries it uses itself, which only
# its pkg-config file lists: OpenSSL for curl, zlib and iconv for libxml2.
# Without them nothing links when only the static libraries are installed.
# vcpkg's CMake configuration brings them itself for MSVC.
MACRO(MB5_USE_STATIC_LDFLAGS LIBRARY PC_MODULE PC_PREFIX VAR)
	IF(NOT MSVC AND "${LIBRARY}" MATCHES "\\.a$" AND NOT "${LIBRARY}" MATCHES "\\.dll\\.a$")
		FIND_PACKAGE(PkgConfig)
		IF(PKG_CONFIG_FOUND)
			PKG_CHECK_MODULES(${PC_PREFIX} QUIET ${PC_MODULE})
			IF(${PC_PREFIX}_FOUND)
				SET(${VAR} ${${PC_PREFIX}_STATIC_LDFLAGS})
			ENDIF(${PC_PREFIX}_FOUND)
		ENDIF(PKG_CONFIG_FOUND)
	ENDIF()
ENDMACRO(MB5_USE_STATIC_LDFLAGS)
MB5_USE_STATIC_LDFLAGS("${CURL_LIBRARY}" libcurl MB5_PC_CURL CURL_LIBRARIES)
MB5_USE_STATIC_LDFLAGS("${LIBXML2_LIBRARIES}" libxml-2.0 MB5_PC_LIBXML2 LIBXML2_LIBRARIES)

SET(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
SET(EXEC_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Installation prefix for executables and object code libraries" FORCE)
SET(BIN_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/bin CACHE PATH "Installation prefix for user executables" FORCE)
SET(LIB_INSTALL_DIR ${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH  "Installation prefix for object code libraries" FORCE)
SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Installation prefix for C header files" FORCE)

# A program in C linking the static C++ library needs the C++ runtime,
# which only the C++ compiler links by itself: list the libraries it adds
# beyond the C compiler's (-lstdc++ with GCC, -lc++ on macOS)
SET(MB5_CXX_RUNTIME_LIBS "")
FOREACH(LIB ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES})
	LIST(FIND CMAKE_C_IMPLICIT_LINK_LIBRARIES "${LIB}" MB5_IN_C)
	# The compiler's own runtime is linked by any compiler driver, and
	# libgcc_s has no static library to link a static program with
	IF(MB5_IN_C EQUAL -1 AND NOT LIB MATCHES "^(gcc|gcc_s|gcc_eh|c)$|_asneeded$")
		IF(LIB MATCHES "^-" OR IS_ABSOLUTE "${LIB}")
			SET(MB5_CXX_RUNTIME_LIBS "${MB5_CXX_RUNTIME_LIBS} ${LIB}")
		ELSE()
			SET(MB5_CXX_RUNTIME_LIBS "${MB5_CXX_RUNTIME_LIBS} -l${LIB}")
		ENDIF()
	ENDIF()
ENDFOREACH(LIB)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libmusicbrainz5.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libmusicbrainz5.pc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libmusicbrainz5cc.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libmusicbrainz5cc.pc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)

FILE(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/include/musicbrainz5/*.h)
INSTALL(FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/include/musicbrainz5/mb5_c.h DESTINATION ${INCLUDE_INSTALL_DIR}/musicbrainz5)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libmusicbrainz5.pc ${CMAKE_CURRENT_BINARY_DIR}/libmusicbrainz5cc.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(examples)

ADD_CUSTOM_TARGET(docs
	doxygen
)

ADD_DEPENDENCIES(docs src_gen)

ADD_CUSTOM_TARGET(dist
    rm -rf "libmusicbrainz-${PROJECT_VERSION}"
    COMMAND git archive --prefix="libmusicbrainz-${PROJECT_VERSION}/" master | gzip -9 > libmusicbrainz-${PROJECT_VERSION}.tar.gz
)
