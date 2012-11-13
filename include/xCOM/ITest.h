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

#ifndef XCOM_ITEST_H
#define XCOM_ITEST_H

#include <xCOM.h>

#define XC_ITEST_NAME          "xcom.ITest"
#define XC_ITEST_VERSION_MAJOR 1
#define XC_ITEST_VERSION_MINOR 0

typedef struct xCOM_ITest {
   XC_INTERFACE;
   xc_result_t (* run) (void);
} xc_itest_t;

#endif /* XCOM_ITEST_H */

