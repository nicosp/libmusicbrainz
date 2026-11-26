/* --------------------------------------------------------------------------

   libmusicbrainz5 - Client library to access MusicBrainz

   Copyright (C) 2026 Nicos Panayides

   This file is part of libmusicbrainz5.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   libmusicbrainz5 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.

----------------------------------------------------------------------------*/

/*
 * Marks which classes and functions libmusicbrainz5cc (the C++ library)
 * and libmusicbrainz5 (the C interface) export from a shared library.
 *
 * Each public class or function carries MB5_CC_PUBLIC or MB5_C_PUBLIC,
 * for the library that defines it.  While that library is being built
 * the macro exports the symbol: __declspec(dllexport) on Windows,
 * default visibility elsewhere (the libraries are compiled with
 * -fvisibility=hidden, so nothing unmarked is exported).  In programs
 * that use the library, MSVC needs __declspec(dllimport) to use a
 * class's static data or vtable from a DLL, so the macro adds it there.
 * MinGW and Cygwin import from a DLL by themselves, so programs built
 * with them link the static or the shared library alike.
 *
 * MB5_CC_INLINE marks classes and class templates whose members are all
 * defined in the headers.  Every program compiles them itself, so on
 * Windows nothing is exported or imported for them (MSVC matches the
 * exceptions a DLL throws by name); elsewhere they get default
 * visibility, so the library and programs share one vtable and typeinfo
 * for each of them.
 *
 * With MSVC, define MB5_STATIC before including any libmusicbrainz5
 * header when linking the static libraries.  It makes no difference with
 * other compilers.
 */

#ifndef _MUSICBRAINZ5_EXPORT_H
#define _MUSICBRAINZ5_EXPORT_H

#if defined(_WIN32) || defined(__CYGWIN__)
#  define MB5_EXPORT __declspec(dllexport)
#  if defined(_MSC_VER)
#    define MB5_IMPORT __declspec(dllimport)
#  else
     /* The MinGW and Cygwin linkers import from a DLL by themselves. */
#    define MB5_IMPORT
#  endif
#  define MB5_CC_INLINE
#elif defined(__GNUC__) && __GNUC__ >= 4
#  define MB5_EXPORT __attribute__((visibility("default")))
#  define MB5_IMPORT __attribute__((visibility("default")))
#  define MB5_CC_INLINE __attribute__((visibility("default")))
#else
#  define MB5_EXPORT
#  define MB5_IMPORT
#  define MB5_CC_INLINE
#endif

#if defined(MB5_STATIC)
#  define MB5_CC_PUBLIC
#  define MB5_C_PUBLIC
#else
#  if defined(MB5_BUILDING_CC)
#    define MB5_CC_PUBLIC MB5_EXPORT
#  else
#    define MB5_CC_PUBLIC MB5_IMPORT
#  endif
#  if defined(MB5_BUILDING_C)
#    define MB5_C_PUBLIC MB5_EXPORT
#  else
#    define MB5_C_PUBLIC MB5_IMPORT
#  endif
#endif

#endif
