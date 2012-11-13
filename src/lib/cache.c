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

#define  TRACE_CLASS_DEFAULT CACHE
#include "internal.h"

#include <expat.h>

static int
mkdir_p (
   const char *path,
   mode_t mode
) {
   int result = mkdir (path, mode);
   if ((result == -1) && (errno == EEXIST)) result=0;
   if (result != 0) {
      int e = errno;
      TRACE1 (("%s: failed to mkdir (%s)", path, sys_errlist[e]));
   }
   return result;
}

void
cache_write_port (
   FILE *cacheFile,
   port_t *portPtr
) {

   TRACE3 (("called with cacheFile=%p, portPtr=%p", cacheFile, portPtr));
   assert (cacheFile != NULL);
   assert (portPtr != NULL);

   fprintf (cacheFile,
      "<port name='%s' interface='%s' vmajor='%u' vminor='%u' size='%u' "
      "proto='%s' flags='%u' component='%s'></port>",
      (portPtr->name != NULL) ? portPtr->name : "",
      (portPtr->interfaceSpec.name != NULL) ? portPtr->interfaceSpec.name : "",
      portPtr->interfaceSpec.vmajor,
      portPtr->interfaceSpec.vminor,
      portPtr->interfaceSize,
      (portPtr->proto != NULL) ? portPtr->proto : "",
      portPtr->flags,
      (portPtr->componentName != NULL) ? portPtr->componentName : ""
   );

   TRACE3 (("exiting"));
}

void
cache_write_ports (
   FILE *cacheFile,
   component_t *componentPtr
) {
   unsigned int i;

   TRACE3 (("called with cacheFile=%p, componentPtr=%p", cacheFile, componentPtr));
   assert (cacheFile != NULL);
   assert (componentPtr != NULL);

   for (i=0; i<componentPtr->portsCount; i++) {
      cache_write_port (cacheFile, componentPtr->ports[i]);
   }

   TRACE3 (("exiting"));
}

xc_result_t
cache_create (
   component_t *componentPtr,
   const char *name
) {
   FILE *cacheFile;
   char path[PATH_MAX];
   xc_result_t result = XC_ERR_INVAL;
   int r;

   TRACE3 (("called with componentPtr=%p, name='%s'", componentPtr, name));
   assert (componentPtr != NULL);
   assert (name != NULL);

   sprintf (path, "%s/" CACHE_FOLDER "/", componentPtr->bundlePtr->path);
   r = mkdir_p (path, 0755);
   if (r == 0) {
      strcat (path, XC_HOST "/");
      r = mkdir_p (path, 0755);
      if (r == 0) {
         strcat (path, name);
         strcat (path, ".xml");
         cacheFile = fopen (path, "w");
         if (cacheFile != NULL) {
            /* Write component meta-data */
            fprintf (cacheFile,
               "<xml>"
               "<component name='%s' descr='%s' vmajor='%u' vminor='%u' ports='%u'>",
               (componentPtr->name != NULL)  ? componentPtr->name  : "",
               (componentPtr->descr != NULL) ? componentPtr->descr : "",
               componentPtr->vmajor,
               componentPtr->vminor,
               componentPtr->portsCount
            );
            /* Write ports meta-data */
            cache_write_ports (cacheFile, componentPtr);
            /* Write footer. */
            fprintf (cacheFile,
               "</component>"
               "</xml>"
            );
            fclose (cacheFile);
            result = XC_OK;
         }
         else {
            TRACE1 (("%s: failed to open for writing!", path));
         }
      }
   }

   TRACE3 (("exiting with result=%d", result));
   return result;
}

static char *
strdup_p (
   const char *str
) {
   if (str [0] == '\0') return NULL;
   return strdup (str);
}

static void
handle_cache_start_tag (
   void *userData,
   const XML_Char *name,
   const XML_Char **attr
) {
   component_t *componentPtr = userData;
   port_t *portPtr;
   unsigned int i;

   TRACE3 (("called with userData=%p, name='%s', atts=%p", userData, name, attr));
   assert (userData != NULL);

   if (strcmp (name, "component") == 0) {
      for (i=0; attr[i]; i+=2) {
         if (strcmp (attr[i], "name") == 0) component_set_name (componentPtr, attr[i+1]);
         else if (strcmp (attr[i], "descr") == 0) component_set_descr (componentPtr, attr[i+1]);
         else if (strcmp (attr[i], "vmajor") == 0) componentPtr->vmajor = atoi (attr[i+1]);
         else if (strcmp (attr[i], "vminor") == 0) componentPtr->vminor = atoi (attr[i+1]);
         else if (strcmp (attr[i], "ports") == 0) {
            componentPtr->portsCount = atoi (attr[i+1]);
            componentPtr->ports = malloc (sizeof (port_t *) * componentPtr->portsCount);
            memset (componentPtr->ports, 0, sizeof (port_t *) * componentPtr->portsCount);
         }
      }
   }
   else if (strcmp (name, "port") == 0) {
      portPtr = malloc (sizeof (*portPtr));
      if (portPtr != NULL) {
         memset (portPtr, 0, sizeof (*portPtr));
         portPtr->componentHandle = componentPtr->id;
         for (i=0; attr[i]; i+=2) {
            if (strcmp (attr[i], "name") == 0)
               portPtr->name = strdup_p (attr[i+1]);
            else if (strcmp (attr[i], "interface") == 0)
               portPtr->interfaceSpec.name = strdup_p (attr[i+1]);
            else if (strcmp (attr[i], "vmajor") == 0)
               portPtr->interfaceSpec.vmajor = atoi (attr[i+1]);
            else if (strcmp (attr[i], "vminor") == 0)
               portPtr->interfaceSpec.vminor = atoi (attr[i+1]);
            else if (strcmp (attr[i], "size") == 0)
               portPtr->interfaceSize = atoi (attr[i+1]);
            else if (strcmp (attr[i], "proto") == 0)
               portPtr->proto = strdup_p (attr[i+1]);
            else if (strcmp (attr[i], "flags") == 0)
               portPtr->flags = atoi (attr[i+1]);
            else if (strcmp (attr[i], "compoent") == 0)
               portPtr->componentName = strdup_p (attr[i+1]);
         }
         for (i=0; componentPtr->ports[i] && i<componentPtr->portsCount; i++);
         if (i<componentPtr->portsCount) {
            componentPtr->ports[i] = portPtr;
            if (XC_PORTF_IS_PROVIDED (portPtr->flags)) {
               provided_ports_register (portPtr);
            }
         }
      }
   }

   TRACE3 (("exiting"));
}

component_t *
cache_open (
   bundle_t *bundlePtr,
   const char *name
) {
   XML_Parser parser;
   component_t *componentPtr = NULL;
   char *path = NULL;
   char buffer [256];
   xc_result_t result = XC_ERR_INVAL;
   int fd;

   TRACE3 (("called with bundlePtr=%p", bundlePtr));
   assert (bundlePtr != NULL);

   path = malloc (PATH_MAX);
   if (path != NULL) {
      sprintf (path, "%s/" CACHE_FOLDER "/" XC_HOST "/%s.xml", bundlePtr->path, name);
      fd = open (path, O_RDONLY);
      if (fd >= 0) {
         parser = XML_ParserCreate (NULL);
         if (parser != NULL) {
            componentPtr = component_new (bundlePtr);
            if (componentPtr != NULL) {
               sprintf (path, "%s/" CODE_FOLDER "/" XC_HOST "/%s", bundlePtr->path, name);
               componentPtr->path = path;
               path = NULL;
               XML_SetUserData (parser, componentPtr);
               XML_SetElementHandler (parser, handle_cache_start_tag, NULL);
               for (;;) {
                  int n = read (fd, buffer, sizeof (buffer));
                  if (n < 0) break;
                  if (!XML_Parse (parser, buffer, n, n==0)) break;
                  if (n==0) {
                     TRACE4 (("%s: loaded meta-data from cache!", componentPtr->name));
                     result = XC_OK;
                     break;
                  }
               }
            }
            XML_ParserFree (parser);
         }
         close (fd);
      }
      else {
         TRACE2 (("%s: failed to open", path));
      }
   }
   
   if (result != XC_OK) {
      if (componentPtr != NULL) {
         component_free (componentPtr);
         componentPtr = NULL;
      }
      free (path);
   }

   TRACE3 (("exiting with result=%p", componentPtr));
   return componentPtr;
}

