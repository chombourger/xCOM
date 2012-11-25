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

#ifndef INTERNAL_H
#define INTERNAL_H

#define _GNU_SOURCE 1

#ifndef  TRACE_CLASS_DEFAULT
#define  TRACE_CLASS_DEFAULT XCOM
#endif

#define CACHE_FOLDER "Cache"
#define CODE_FOLDER  "Code"

#define  TRACE_ENV_PREFIX    "XCOM_TRACE_"
#define  TRACE_TRC_FILE      "xcom.trc.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <xCOM.h>

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include <semaphore.h>

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_STDIO_H 
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef GLIB_ENABLED
#include <glib.h>
#endif

typedef enum ImportState {
   IMPORT_STATE_QUERIED,
   IMPORT_STATE_UNREGISTERED,
   IMPORT_STATE_REGISTERING,
   IMPORT_STATE_REGISTERED,
   IMPORT_STATE_UNREGISTERING
} import_state_t;

struct Bundle;
struct Component;
struct Import;
struct Provider;

typedef struct Bundle    bundle_t;
typedef struct Component component_t;
typedef struct Import    import_t;
typedef struct Port      port_t;

#include "cache.h"
#include "component.h"
#include "handledir.h"
#include "hashtable.h"
#include "import.h"
#include "interfaces.h"
#include "ports.h"
#include "query.h"
#include "lock.h"
#include "trace.h"

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#endif /* INTERNAL_H */

