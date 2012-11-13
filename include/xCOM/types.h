/*
 * xCOM - a Simple Component Framework
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef XCOM_TYPES_H
#define XCOM_TYPES_H

#include <xCOM/clist.h>
#include <stddef.h>

#ifdef __XCOM__

#ifdef HAVE_VISIBILITY
#define XCOM_EXPORT __attribute__((visibility("default")))
#endif

#endif /* __XCOM__ */

#ifndef XCOM_EXPORT
#define XCOM_EXPORT
#endif

#define XCOM_INTERFACE(name,vmaj,vmin) { name, vmaj, vmin }

/** xCOM Method Result type. A value greater or equal to zero indicates
  * success, a negative value an error. */
typedef int xc_result_t;

/** Success result code. */
#define XC_OK ((xc_result_t) 0)

/** Not Enough Memory. */
#define XC_ERR_NOMEM ((xc_result_t) 1)

/** Invalid value error. */
#define XC_ERR_INVAL ((xc_result_t) 2)

/** No such entry error. */
#define XC_ERR_NOENT ((xc_result_t) 3)

/** Not a directory error. */
#define XC_ERR_NODIR ((xc_result_t) 4)

/** Resource already exists. */
#define XC_ERR_EXIST ((xc_result_t) 5)

/** Resource is busy. */
#define XC_ERR_BUSY ((xc_result_t) 6)

/** Key associated to xCOM objects to avoid passing pointers. */
typedef unsigned int xc_handle_t;

/** Invalid handle number. */
#define XC_INVALID_HANDLE ((xc_handle_t)0)

#endif /* XCOM_H */

