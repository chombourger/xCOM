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

#define  TRACE_CLASS_DEFAULT SEMAPHORE
#include "internal.h"

#if defined(HAVE_MACH_SEMAPHORE_H)
# include <mach/semaphore.h>
# if defined(HAVE_MACH_TASK_H)
#  include <mach/task.h>
#  include <mach/mach_init.h>
#  define USE_MACH_SEMAPHORE 1
# endif
#elif defined(HAVE_SEMAPHORE_H)
# define USE_POSIX_SEMAPHORE 1
# include <semaphore.h>
#else
# error "unsupported OS!"
#endif

#ifdef USE_POSIX_SEMAPHORE

xc_result_t
xc_sem_init (
   xc_sem_t *semPtr,
   int value
) {
   xc_result_t result;
   int err;

   TRACE3 (("called with semPtr=%p, value=%d", semPtr, value));

   if (sizeof (xc_sem_t) >= sizeof (sem_t)) {
      result = sem_init ((sem_t *) semPtr, 0, value);
      if (result == 0) {
         result = XC_OK;
      }
      else {
         err = errno;
         TRACE1 (("sem_init() failed: %s", strerror (err)));
         switch (err) {
            case EINVAL: result = XC_ERR_INVAL;   break;
            default    : result = XC_ERR_UNKNOWN; break;
         }
      }
   }
   else {
      TRACE1 (("internal error: sizeof(xc_sem_t) shall be >= %d", sizeof (sem_t)));
      result = XC_ERR_INTERNAL;
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xc_sem_destroy (
   xc_sem_t *semPtr
) {
   xc_result_t result;
   int err;

   TRACE3 (("called with semPtr=%p", semPtr));
   result = sem_destroy ((sem_t *) semPtr);

   if (result == 0) {
      result = XC_OK;
   }
   else {
      err = errno;
      TRACE1 (("sem_destroy() failed: %s", strerror (err)));
      switch (errno) {
         case EINVAL: result = XC_ERR_INVAL;   break;
         default    : result = XC_ERR_UNKNOWN; break;
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xc_sem_signal (
   xc_sem_t *semPtr
) {
   xc_result_t result;
   int err;

   TRACE3 (("called with semPtr=%p", semPtr));
   result = sem_post ((sem_t *) semPtr);

   if (result == 0) {
      result = XC_OK;
   }
   else {
      err = errno;
      TRACE1 (("sem_post() failed: %s", strerror (err)));
      switch (errno) {
         case EINVAL: result = XC_ERR_INVAL;   break;
         default    : result = XC_ERR_UNKNOWN; break;
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

xc_result_t
xc_sem_wait (
   xc_sem_t *semPtr
) {
   xc_result_t result;
   int err;

   TRACE3 (("called with semPtr=%p", semPtr));
   do {
      result = sem_wait ((sem_t *) semPtr);
   } while ((result == -1) && (errno == EINTR));

   if (result == 0) {
      result = XC_OK;
   }
   else {
      err = errno;
      TRACE1 (("sem_wait() failed: %s", strerror (err)));
      switch (errno) {
         case EINVAL: result = XC_ERR_INVAL;   break;
         default    : result = XC_ERR_UNKNOWN; break;
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

#endif /* USE_POSIX_SEMAPHORE */

#ifdef USE_MACH_SEMAPHORE

xc_result_t
xc_sem_init (
   xc_sem_t *semPtr,
   int value
) {

   if (sizeof (xc_sem_t) > sizeof (semaphore_t)) {
      kern_return_t result = semaphore_create (
         mach_task_self (),
         (semaphore_t *) semPtr,
         SYNC_POLICY_FIFO,
         value
      );
      if (result == 0) {
         return XC_OK;
      }
      switch (errno) {
         case EINVAL: return XC_ERR_INVAL;
         default    : return XC_ERR_UNKNOWN;
      }
   }
   return XC_ERR_INTERNAL;
}

xc_result_t
xc_sem_destroy (
   xc_sem_t *semPtr
) {
   kern_return_t result = semaphore_destroy (
      mach_task_self (),
      *(semaphore_t *) semPtr
   );
   if (result == 0) {
      return XC_OK;
   }
   switch (errno) {
      case EINVAL: return XC_ERR_INVAL;
      default    : return XC_ERR_UNKNOWN;
   }
}

xc_result_t
xc_sem_wait (
   xc_sem_t *semPtr
) {
   kern_return_t result = semaphore_wait (*(semaphore_t *) semPtr);
   if (result == 0) {
      return XC_OK;
   }
   switch (errno) {
      case EINVAL: return XC_ERR_INVAL;
      default    : return XC_ERR_UNKNOWN;
   }
}

xc_result_t
xc_sem_signal (
   xc_sem_t *semPtr
) {
   kern_return_t result = semaphore_signal (*(semaphore_t *) semPtr);
   if (result == 0) {
      return XC_OK;
   }
   switch (errno) {
      case EINVAL: return XC_ERR_INVAL;
      default    : return XC_ERR_UNKNOWN;
   }
}

#endif /* USE_MACH_SEMAPHORE */

