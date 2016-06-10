/** common.h
 *
 *  @copyright
 *  Copyright (C) 2010-2013
 *  Intel Corporation
 *  
 *  @copyright
 *  This file is part of the Intel Cilk Plus Library.  This library is free
 *  software; you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *  
 *  @copyright
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  @copyright
 *  Under Section 7 of GPL version 3, you are granted additional
 *  permissions described in the GCC Runtime Library Exception, version
 *  3.1, as published by the Free Software Foundation.
 *  
 *  @copyright
 *  You should have received a copy of the GNU General Public License and
 *  a copy of the GCC Runtime Library Exception along with this program;
 *  see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
 *  <http://www.gnu.org/licenses/>.
 */
 
/** @file common.h
 *
 * @brief Defines common macros and structures used by the Intel Cilk Plus
 * runtime.
 *
 *  @ingroup common
 */

/** @defgroup common Common Definitions
 *  Macro, structure, and class definitions used elsewhere in the runtime.
 *  @{
 */
 
#ifndef INCLUDED_CILK_COMMON
#define INCLUDED_CILK_COMMON

#ifdef __cplusplus
/** Namespace for all Cilk definitions that can be included in user code.
 */
namespace cilk {
    
    /** Namespace for definitions that are primarily intended for use
     *  in other Cilk definitions.
     */
    namespace internal {}
}
#endif

/** Cilk library version = 1.01
 */
#define CILK_LIBRARY_VERSION 101

#ifdef __cplusplus
#   include <cassert>
#else
#   include <assert.h>
#endif

/**
 * Prefix standard library function and type names with __STDNS in order to
 * get correct lookup in both C and C++.
 */
#ifdef __cplusplus
#   define __STDNS std::
#else
#   define __STDNS
#endif

/**
 * @def CILK_EXPORT
 * Define export of runtime functions from shared library.
 * Should be exported only from cilkrts*.dll/cilkrts*.so
 * @def CILK_EXPORT_DATA
 * Define export of runtime data from shared library.
 */
#ifdef _WIN32
#   ifdef IN_CILK_RUNTIME
#       define CILK_EXPORT      __declspec(dllexport)
#       define CILK_EXPORT_DATA __declspec(dllexport)
#   else
#       define CILK_EXPORT      __declspec(dllimport)
#       define CILK_EXPORT_DATA __declspec(dllimport)
#   endif  /* IN_CILK_RUNTIME */
#elif defined(__CYGWIN__) || defined(__APPLE__) || defined(_DARWIN_C_SOURCE)
#   define CILK_EXPORT      /* nothing */
#   define CILK_EXPORT_DATA /* nothing */
#else /* Unix/gcc */
#   ifdef IN_CILK_RUNTIME
#       define CILK_EXPORT      __attribute__((visibility("protected")))
#       define CILK_EXPORT_DATA __attribute__((visibility("protected")))
#   else
#       define CILK_EXPORT      /* nothing */
#       define CILK_EXPORT_DATA /* nothing */
#   endif  /* IN_CILK_RUNTIME */
#endif /* Unix/gcc */

/**
 * @def __CILKRTS_BEGIN_EXTERN_C
 * Macro to denote the start of a section in which all names have "C" linkage.
 * That is, none of the names are to be mangled.
 * @see __CILKRTS_END_EXTERN_C
 * @see __CILKRTS_EXTERN_C
 *
 * @def __CILKRTS_END_EXTERN_C
 * Macro to denote the end of a section in which all names have "C" linkage.
 * That is, none of the names are to be mangled.
 * @see __CILKRTS_BEGIN_EXTERN_C
 * @see __CILKRTS_EXTERN_C
 *
 * @def __CILKRTS_EXTERN_C
 * Macro to prefix a single definition which has "C" linkage.
 * That is, the defined name is not to be mangled.
 * @see __CILKRTS_BEGIN_EXTERN_C
 * @see __CILKRTS_END_EXTERN_C
 */
#ifdef __cplusplus
#   define __CILKRTS_BEGIN_EXTERN_C     extern "C" {
#   define __CILKRTS_END_EXTERN_C       }
#   define __CILKRTS_EXTERN_C           extern "C"
#else
#   define __CILKRTS_BEGIN_EXTERN_C
#   define __CILKRTS_END_EXTERN_C
#   define __CILKRTS_EXTERN_C
#endif

/**
 * OS-independent macro to specify a function which is known to not throw
 * an exception.
 */ 
#ifdef __cplusplus
#   ifdef _WIN32
#       define __CILKRTS_NOTHROW __declspec(nothrow)
#   else /* Unix/gcc */
#       define __CILKRTS_NOTHROW __attribute__((nothrow))
#   endif /* Unix/gcc */
#else
#   define __CILKRTS_NOTHROW /* nothing */
#endif /* __cplusplus */

/** Cache alignment. (Good enough for most architectures.)
 */
#define __CILKRTS_CACHE_LINE__ 64

/**
 * Macro to specify alignment of a data member in a structure.
 * Because of the way that gcc’s alignment attribute is defined, @a n must
 * be a numeric literal, not just a compile-time constant expression.
 */
#ifdef _WIN32
#   define CILK_ALIGNAS(n) __declspec(align(n))
#else /* Unix/gcc */
#   define CILK_ALIGNAS(n) __attribute__((__aligned__(n)))
#endif

/**
 * Macro to specify cache-line alignment of a data member in a structure.
 */
#define __CILKRTS_CACHE_ALIGN CILK_ALIGNAS(__CILKRTS_CACHE_LINE__)

/**
 * Macro to specify a class as being at least as strictly aligned as some
 * type on Windows. gcc does not provide a way of doing this, so on Unix, 
 * this just specifies the largest natural type alignment. Put the macro
 * between the `class` keyword and the class name:
 *
 *      class CILK_ALIGNAS_TYPE(foo) bar { ... };
 */
#ifdef _WIN32
#   define CILK_ALIGNAS_TYPE(t) __declspec(align(__alignof(t)))
#else /* Unix/gcc */
#   define CILK_ALIGNAS_TYPE(t) __attribute__((__aligned__))
#endif

/**
 * @def CILK_API(RET_TYPE)
 * A function called explicitly by the programmer.
 * @def CILK_ABI(RET_TYPE)
 * A function called by compiler-generated code.
 * @def CILK_ABI_THROWS(RET_TYPE)
 * An ABI function that may throw an exception
 *
 * Even when these are the same definitions, they should be separate macros so
 * that they can be easily found in the code.
 */

#ifdef _WIN32
#   define CILK_API(RET_TYPE) CILK_EXPORT RET_TYPE __CILKRTS_NOTHROW __cdecl
#   define CILK_ABI(RET_TYPE) CILK_EXPORT RET_TYPE __CILKRTS_NOTHROW __cdecl
#   define CILK_ABI_THROWS(RET_TYPE) CILK_EXPORT RET_TYPE __cdecl
#else
#   define CILK_API(RET_TYPE) CILK_EXPORT RET_TYPE __CILKRTS_NOTHROW
#   define CILK_ABI(RET_TYPE) CILK_EXPORT RET_TYPE __CILKRTS_NOTHROW
#   define CILK_ABI_THROWS(RET_TYPE) CILK_EXPORT RET_TYPE
#endif

/**
 * __CILKRTS_ASSERT should be defined for debugging only, otherwise it
 * interferes with vectorization.  Since NDEBUG is not reliable (it must be
 * set by the user), we must use a platform-specific detection of debug mode.
 */
#if defined(_WIN32) && defined(_DEBUG)
    /* Windows debug */
#   define __CILKRTS_ASSERT(e) assert(e)
#elif (! defined(_WIN32)) && ! defined(__OPTIMIZE__)
    /* Unix non-optimized */
#   define __CILKRTS_ASSERT(e) assert(e)
#elif defined __cplusplus
    /* C++ non-debug */
#   define __CILKRTS_ASSERT(e) static_cast<void>(0)
#else
    /* C non-debug */
#   define __CILKRTS_ASSERT(e) ((void) 0)
#endif

/**
 * OS-independent macro to specify a function that should be inlined
 */
#ifdef __cpluspus
    // C++
#   define __CILKRTS_INLINE inline
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
    // C99
#   define __CILKRTS_INLINE static inline
#elif defined(_MSC_VER)
    // C89 on Windows
#   define __CILKRTS_INLINE __inline
#else
    // C89 on GCC-compatible systems
#   define __CILKRTS_INLINE extern __inline__
#endif

/**
 * Functions marked as CILK_EXPORT_AND_INLINE have both
 * inline versions defined in the Cilk API, as well as
 * non-inlined versions that are exported (for
 * compatibility with previous versions that did not
 * inline the functions).
 */
#ifdef COMPILING_CILK_API_FUNCTIONS
#   define CILK_EXPORT_AND_INLINE  CILK_EXPORT
#else
#   define CILK_EXPORT_AND_INLINE  __CILKRTS_INLINE
#endif

/**
 * Try to determine if compiler supports rvalue references.
 */
#if defined(__cplusplus) && !defined(__CILKRTS_RVALUE_REFERENCES)
#   if __cplusplus >= 201103L // C++11
#       define __CILKRTS_RVALUE_REFERENCES 1
#   elif defined(__GXX_EXPERIMENTAL_CXX0X__)
#       define __CILKRTS_RVALUE_REFERENCES 1
#   elif __cplusplus >= 199711L && __cplusplus < 201103L
        // Compiler recognizes a language version prior to C++11
#   elif __INTEL_COMPILER == 1200 && defined(__STDC_HOSTED__)
        // Intel compiler version 12.0
        // __cplusplus has a non-standard definition.  In the absence of a
        // proper definition, look for the C++0x macro, __STDC_HOSTED__.
#       define __CILKRTS_RVALUE_REFERENCES 1
#   elif __INTEL_COMPILER > 1200 && defined(CHAR16T)
        // Intel compiler version >= 12.1
        // __cplusplus has a non-standard definition.  In the absence of a
        // proper definition, look for the Intel macro, CHAR16T
#       define __CILKRTS_RVALUE_REFERENCES 1
#   endif
#endif

/*
 * Include stdint.h to define the standard integer types.
 *
 * Unfortunately Microsoft doesn't provide stdint.h until Visual Studio 2010,
 * so use our own definitions until those are available
 */

#if ! defined(_MSC_VER) || (_MSC_VER >= 1600)
#   include <stdint.h>
#else
#   ifndef __MS_STDINT_TYPES_DEFINED__
#       define __MS_STDINT_TYPES_DEFINED__
        typedef signed char int8_t;
        typedef short int16_t;
        typedef int int32_t;
        typedef __int64 int64_t;

        typedef unsigned char uint8_t;
        typedef unsigned short uint16_t;
        typedef unsigned int uint32_t;
        typedef unsigned __int64 uint64_t;
#   endif  /* __MS_STDINT_TYPES_DEFINED__ */
#endif  /* ! defined(_MSC_VER) || (_MSC_VER >= 1600) */

/**
 * @brief Application Binary Interface version of the Cilk runtime library.
 *
 * The ABI version is determined by the compiler used.  An object file
 * compiled with a higher ABI version is not compatible with a library that is
 * compiled with a lower ABI version.  An object file compiled with a lower
 * ABI version, however, can be used with a library compiled with a higher ABI
 * version unless otherwise stated.
 */
#ifndef __CILKRTS_ABI_VERSION
#   ifdef IN_CILK_RUNTIME
#       define __CILKRTS_ABI_VERSION 1
#   elif __INTEL_COMPILER > 1200
        // Intel compiler version >= 12.1
#       define __CILKRTS_ABI_VERSION 1
#   else
        // Compiler does not support ABI version 1
        // (Non-Intel compiler or Intel compiler prior to version 12.1).
#       define __CILKRTS_ABI_VERSION 0
#   endif
#endif

// These structs are exported because the inlining of
// the internal version of API methods require a worker
// structure as parameter. 
__CILKRTS_BEGIN_EXTERN_C
    /// Worker struct, exported for inlined API methods
    /// @ingroup api
    struct __cilkrts_worker;

    /// Worker struct, exported for inlined API methods
    /// @ingroup api
    typedef struct __cilkrts_worker __cilkrts_worker;     

    /// Worker struct pointer, exported for inlined API methods
    /// @ingroup api
    typedef struct __cilkrts_worker *__cilkrts_worker_ptr; 
    
    
    /// Fetch the worker out of TLS.
    CILK_ABI(__cilkrts_worker_ptr) __cilkrts_get_tls_worker(void);

    /// void *, defined to work around complaints from the compiler
    /// about using __declspec(nothrow) after the "void *" return type
    typedef void * __cilkrts_void_ptr;

__CILKRTS_END_EXTERN_C

                                   
#if __CILKRTS_ABI_VERSION >= 1
// Pedigree API is available only for compilers that use ABI version >= 1.

/** Pedigree information kept in the worker and stack frame.
 *  @ingroup api
 */
typedef struct __cilkrts_pedigree
{
    /** Rank at start of spawn helper. Saved rank for spawning functions */
    uint64_t rank;
                                         
    /** Link to next in chain */
    const struct __cilkrts_pedigree *parent;
} __cilkrts_pedigree;

#endif // __CILKRTS_ABI_VERSION >= 1

/// @}

#endif /* INCLUDED_CILK_COMMON */