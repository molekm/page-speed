/*
*******************************************************************************
*
*   Copyright (C) 2002-2009, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  uenum.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:2
*
*   created on: 2002jul08
*   created by: Vladimir Weinstein
*/

#ifndef __UENUM_H
#define __UENUM_H

#include "unicode/utypes.h"
#include "unicode/localpointer.h"

#if U_SHOW_CPLUSPLUS_API
#include "unicode/strenum.h"
#endif

/**
 * \file
 * \brief C API: String Enumeration 
 */
 
/**
 * An enumeration object.
 * For usage in C programs.
 * @stable ICU 2.2
 */
struct UEnumeration;
/** structure representing an enumeration object instance @stable ICU 2.2 */
typedef struct UEnumeration UEnumeration;

/**
 * Disposes of resources in use by the iterator.  If en is NULL,
 * does nothing.  After this call, any char* or UChar* pointer
 * returned by uenum_unext() or uenum_next() is invalid.
 * @param en UEnumeration structure pointer
 * @stable ICU 2.2
 */
U_STABLE void U_EXPORT2
uenum_close(UEnumeration* en);

#if U_SHOW_CPLUSPLUS_API

U_NAMESPACE_BEGIN

/**
 * \class LocalUEnumerationPointer
 * "Smart pointer" class, closes a UEnumeration via uenum_close().
 * For most methods see the LocalPointerBase base class.
 *
 * @see LocalPointerBase
 * @see LocalPointer
 * @draft ICU 4.4
 */
U_DEFINE_LOCAL_OPEN_POINTER(LocalUEnumerationPointer, UEnumeration, uenum_close);

U_NAMESPACE_END

#endif

/**
 * Returns the number of elements that the iterator traverses.  If
 * the iterator is out-of-sync with its service, status is set to
 * U_ENUM_OUT_OF_SYNC_ERROR.
 * This is a convenience function. It can end up being very
 * expensive as all the items might have to be pre-fetched (depending
 * on the type of data being traversed). Use with caution and only 
 * when necessary.
 * @param en UEnumeration structure pointer
 * @param status error code, can be U_ENUM_OUT_OF_SYNC_ERROR if the
 *               iterator is out of sync.
 * @return number of elements in the iterator
 * @stable ICU 2.2
 */
U_STABLE int32_t U_EXPORT2
uenum_count(UEnumeration* en, UErrorCode* status);

/**
 * Returns the next element in the iterator's list.  If there are
 * no more elements, returns NULL.  If the iterator is out-of-sync
 * with its service, status is set to U_ENUM_OUT_OF_SYNC_ERROR and
 * NULL is returned.  If the native service string is a char* string,
 * it is converted to UChar* with the invariant converter.
 * The result is terminated by (UChar)0.
 * @param en the iterator object
 * @param resultLength pointer to receive the length of the result
 *                     (not including the terminating \\0).
 *                     If the pointer is NULL it is ignored.
 * @param status the error code, set to U_ENUM_OUT_OF_SYNC_ERROR if
 *               the iterator is out of sync with its service.
 * @return a pointer to the string.  The string will be
 *         zero-terminated.  The return pointer is owned by this iterator
 *         and must not be deleted by the caller.  The pointer is valid
 *         until the next call to any uenum_... method, including
 *         uenum_next() or uenum_unext().  When all strings have been
 *         traversed, returns NULL.
 * @stable ICU 2.2
 */
U_STABLE const UChar* U_EXPORT2
uenum_unext(UEnumeration* en,
            int32_t* resultLength,
            UErrorCode* status);

/**
 * Returns the next element in the iterator's list.  If there are
 * no more elements, returns NULL.  If the iterator is out-of-sync
 * with its service, status is set to U_ENUM_OUT_OF_SYNC_ERROR and
 * NULL is returned.  If the native service string is a UChar*
 * string, it is converted to char* with the invariant converter.
 * The result is terminated by (char)0.  If the conversion fails
 * (because a character cannot be converted) then status is set to
 * U_INVARIANT_CONVERSION_ERROR and the return value is undefined
 * (but non-NULL).
 * @param en the iterator object
 * @param resultLength pointer to receive the length of the result
 *                     (not including the terminating \\0).
 *                     If the pointer is NULL it is ignored.
 * @param status the error code, set to U_ENUM_OUT_OF_SYNC_ERROR if
 *               the iterator is out of sync with its service.  Set to
 *               U_INVARIANT_CONVERSION_ERROR if the underlying native string is
 *               UChar* and conversion to char* with the invariant converter
 *               fails. This error pertains only to current string, so iteration
 *               might be able to continue successfully.
 * @return a pointer to the string.  The string will be
 *         zero-terminated.  The return pointer is owned by this iterator
 *         and must not be deleted by the caller.  The pointer is valid
 *         until the next call to any uenum_... method, including
 *         uenum_next() or uenum_unext().  When all strings have been
 *         traversed, returns NULL.
 * @stable ICU 2.2
 */
U_STABLE const char* U_EXPORT2
uenum_next(UEnumeration* en,
           int32_t* resultLength,
           UErrorCode* status);

/**
 * Resets the iterator to the current list of service IDs.  This
 * re-establishes sync with the service and rewinds the iterator
 * to start at the first element.
 * @param en the iterator object
 * @param status the error code, set to U_ENUM_OUT_OF_SYNC_ERROR if
 *               the iterator is out of sync with its service.  
 * @stable ICU 2.2
 */
U_STABLE void U_EXPORT2
uenum_reset(UEnumeration* en, UErrorCode* status);

#if U_SHOW_CPLUSPLUS_API

/**
 * Given a StringEnumeration, wrap it in a UEnumeration.  The
 * StringEnumeration is adopted; after this call, the caller must not
 * delete it (regardless of error status).
 * @param adopted the C++ StringEnumeration to be wrapped in a UEnumeration.
 * @param ec the error code.
 * @return a UEnumeration wrapping the adopted StringEnumeration.
 * @draft ICU 4.2
 */
U_CAPI UEnumeration* U_EXPORT2
uenum_openFromStringEnumeration(U_NAMESPACE_QUALIFIER StringEnumeration* adopted, UErrorCode* ec);

#endif

#endif
