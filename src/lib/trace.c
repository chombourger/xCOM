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

#include "internal.h"

/* Instantiate __TRACE_CLASS_<x> variables. */
#define  TRACE_CLASS(x) int TRACE_CONCAT2(__TRACE_CLASS_,x) = 0;
#include TRACE_TRC_FILE
#undef   TRACE_CLASS

/* Build a string array of environment variable names. */

struct trace_class {
   const char *name;
   int *p_level;
};

#define  TRACE_CLASS(x) { TRACE_ENV_PREFIX #x, & TRACE_CONCAT2(__TRACE_CLASS_,x) },
static const struct trace_class trace_classes[] = {
#include TRACE_TRC_FILE
#undef   TRACE_CLASS
   { 0, 0 }
};

static pthread_mutex_t trace_lock = PTHREAD_MUTEX_INITIALIZER;
static char trace_buf [256];

void
trace_init (
   void
) {
   const struct trace_class *p_trace_class;
   const char *setting;
   int def = 0;

   setting = getenv (TRACE_ENV_PREFIX "ALL");
   if (setting != NULL) {
      def = (int) strtol (setting, 0, 0);
   }

   for (p_trace_class = trace_classes; p_trace_class->name != 0; p_trace_class ++) {
      int *p_level = p_trace_class->p_level;
      setting = getenv (p_trace_class->name);
      *p_level = def;
      if (setting != NULL) {
         long value = strtol (setting, 0, 0);
         *p_level = (int) (value);
      }
   }
}

void
trace_start (
   const char *klass,
   const char *file,
   int line,
   const char *func
) {
   pthread_mutex_lock (&trace_lock);
   trace (
      "# %-15s %-30s thread %p %s:%d\n"
      "# %-15s ",
      klass, func, pthread_self (), file, line, klass
   );
}

void
trace_end (
   void
) {
   fputc ('\n', stderr);
   pthread_mutex_unlock (&trace_lock);
}

void
trace (
   const char* fmt,
   ...
) {
   va_list args;

   va_start (args, fmt);
   vsnprintf (trace_buf, sizeof (trace_buf), fmt, args);
   trace_buf [sizeof (trace_buf) - 1] = '\0';
   va_end (args);

   fputs (trace_buf, stderr);
}

