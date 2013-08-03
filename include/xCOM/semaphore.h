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

#ifndef XCOM_SEMAPHORE_H
#define XCOM_SEMAPHORE_H

#include <xCOM.h>

xc_result_t
xc_sem_init (
    xc_sem_t *semPtr,
    int       value
)
XCOM_EXPORT;

xc_result_t
xc_sem_destroy (
   xc_sem_t *semPtr
)
XCOM_EXPORT;

xc_result_t
xc_sem_wait (
   xc_sem_t *semPtr
)
XCOM_EXPORT;

xc_result_t
xc_sem_signal (
   xc_sem_t *semPtr
)
XCOM_EXPORT;

#endif /* XCOM_SEMAPHORE_H */

