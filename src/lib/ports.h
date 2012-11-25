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

#ifndef PORTS_H
#define PORTS_H

#ifdef __cplusplus
extern "C" {
#endif

struct ProvidedPortsList {
   pthread_mutex_t lock;
   char *interface;
   xc_clist_t ports;
};

struct Port {
   xc_clist_t node;
   char *name;
   xc_handle_t componentHandle;
   component_t *componentPtr;
   xc_interface_t interfaceSpec;
   const xc_interface_t *interfacePtr;
   unsigned int interfaceSize;
   char *port;
   void *reserved;
   unsigned int flags;
   /* Provided ports */
   xc_register_t onRegister;
   xc_unregister_t onUnRegister;
   /* Required ports */
   char *componentName;
   xc_handle_t *importHandlePtr;
};

/**
  * Initialize the ports module. This method is called by xCOM_Init if
  * the framework wasn't initialized already.
  *
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
ports_module_init (
   void
);

/**
  * Free resources used by the ports module. This is called by xCOM_Destroy
  * when the framework is shutting down. Note that xCOM expects this routine
  * to actually check whether it has been actually optimized.
  *
  */
void
ports_module_destroy (
   void
);

port_t *
port_new (
   xc_handle_t componentHandle,
   struct __xc_port_decl__ *portDeclPtr
);

void
port_free (
   port_t *portPtr
);

struct ProvidedPortsList *
provided_ports_ref (
   const char *name
);

void
provided_ports_unref (
   struct ProvidedPortsList *listPtr
);

xc_result_t
provided_ports_register (
   port_t *portPtr
);

xc_result_t
port_enable (
   port_t *portPtr,
   struct __xc_port_decl__ *portDeclPtr
);

xc_result_t
port_disable (
   port_t *portPtr
);

#ifdef __cplusplus
}
#endif

#endif /* PORTS_H */

