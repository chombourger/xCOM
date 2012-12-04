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

#ifndef XCOM_H
#define XCOM_H

/** @file */

#include <xCOM/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XC_LOADF_NONE          0  /**< Load with no flags */
#define XC_LOADF_IGNORE_CACHES 1  /**< Ignore caches when loading bundle */

#define XC_PORTF_NONE      0  /**< Port with no flags */

#define XC_PORTF_ROLE_MASK 1  /**< Bit mask for the role of the port */
#define XC_PORTF_REQUIRED  0  /**< Port is for a "required interface" */
#define XC_PORTF_PROVIDED  1  /**< Port is for a "provided interface" */

#define XC_PORTF_TYPE_MASK 2  /**< Bit mask for the type of the port */
#define XC_PORTF_RUNTIME   0  /**< "provided interface" will be loaded at runtime */
#define XC_PORTF_LOADTIME  2  /**< "provided interface" to be resolved when component gets loaded */

#define XC_PORTF_OPTIONAL  4  /**< whether the imported interface is optional */
#define XC_PORTF_MULTIPLE  8  /**< whether to get the first or all matches for a loadtime import. */

/** Macro for checking if the port flags indicate a "required interface".
  * @hideinitializer
  */
#define XC_PORTF_IS_REQUIRED(f) ((f & XC_PORTF_ROLE_MASK) == XC_PORTF_REQUIRED)

/** Macro for checking if the port flags indicate a "provided interface." */
#define XC_PORTF_IS_PROVIDED(f) ((f & XC_PORTF_ROLE_MASK) == XC_PORTF_PROVIDED)

/** Macro for checking if the port is to be resolved at loadtime. */
#define XC_PORTF_IS_LOADTIME(f) ((f & XC_PORTF_TYPE_MASK) == XC_PORTF_LOADTIME)
/** Macro for checking if the port will be resolved at runtime. */
#define XC_PORTF_IS_RUNTIME(f) ((f & XC_PORTF_TYPE_MASK) == XC_PORTF_RUNTIME)

/** Macro for checking if the import is optional. */
#define XC_PORTF_IS_OPTIONAL(f) ((f & XC_PORTF_OPTIONAL) != 0)

/** Macro for checking if the import is multiple. */
#define XC_PORTF_IS_MULTIPLE(f) ((f & XC_PORTF_MULTIPLE) != 0)

/** No query flags defined at this point. */
#define XC_QUERYF_NONE    0

/**
  * Structure for declaring an xCOM interface. Interfaces have a name
  * and a version number. The major version number shall be bumped
  * whenever binary compatibility with former versions is broken (e.g.,
  * member removed). Minor number should be changed whenever the
  * interface is changed but in a way where old clients are not
  * affected (e.g., member added at the end).
  *
  */
typedef struct {
   const char *name;    /**< Name of the interface. */
   unsigned int vmajor; /**< Major version number of the interface. */
   unsigned int vminor; /**< Minor version number of the interface. */
} xc_interface_t;

/**
  * Prototype of the interface register function. When a component
  * exposes a "provided interface" port, the register() method will
  * be called by the xCOM framework to check first with the server
  * component is the connection should be established. This can
  * be used for a server implemented in such a way where it can
  * service a limited number of clients. Note that register() may
  * be called before the init() method of the component. This allows
  * a server to deny a connection before its loadtime dependencies
  * are resolved.
  *
  * The register function is called with two arguments: the handle
  * of its component and the handle of the import. The import handle
  * can be used to obtain information even before the connection
  * with the client is established.
  *
  */
typedef xc_result_t (* xc_register_t) (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
);

/**
  * Prototype of the interface unregister function. When the
  * import of a "provided interface" is closed, the xCOM framework
  * would call the unregister() method declared for this port. This
  * allows a service to clean-up things it would have for instance
  * allocated for a specific client.
  *
  * The unregister method is called with two arguments: the server
  * component handle as well as the import handle.
  *
  */
typedef xc_result_t (* xc_unregister_t) (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
);

/**
  * @internal
  *
  * Structure describing an xCOM port. Note xCOM components must not
  * use this structure directly as it may change in future versions
  * of the framework.
  *
  */
struct __xc_port_decl__ {
   const char *name;                   /**< Name of the port. */
   const xc_interface_t *interfacePtr; /**< Pointer to the interface declaration. */
   unsigned int interfaceSize;         /**< Size of the interface declaration. */
   const char *component;              /**< Name of the queried component. */
   const char *port;                   /**< Queried port or NULL for any. */
   void *reserved;                     /**< Reserved for future use. */
   unsigned int flags;                 /**< xCOM port flags, see XCOM_PORTF_ defines. */
   const xc_register_t onRegister;     /**< Register function for a "provided interface" port. */
   const xc_unregister_t onUnRegister; /**< Unregister function for a "required interface" port. */
   xc_handle_t *importHandlePtr;       /**< Where the xCOM framework would store the import handle. */
};

/** component init method. */
typedef xc_result_t (* xc_init_t) (xc_handle_t componentHandle);

/** component destroy method. */
typedef xc_result_t (* xc_destroy_t) (xc_handle_t componentHandle);

/** 
  * Structure used by the XC_DECLARE_COMPONENT macro to describe
  * an xCOM component in a dynamic shared object.
  *
  */
struct __xc_component_decl__ {
   const char *name;                /**< Name of the component. */
   const char *descr;               /**< Short description of the component. */
   unsigned int vmaj;               /**< Major version number of the component. */
   unsigned int vmin;               /**< Minor version number of the component. */
   xc_init_t init;                  /**< Component init() method (if any). */
   xc_destroy_t destroy;            /**< Component destroy() method (if any). */
   struct __xc_port_decl__ ports[]; /**< Array of ports either provided or required. */
};

/** Name of the declared component symbol.
  * @hideinitializer
  */
#define XC_DECLARED_COMPONENT_SYM __xc_component_decl__

/** 
  * Declare an xCOM component. The macro should be used exactly once in a given
  * dynamic shared object and should be used as follow:
  *
  * @hideinitializer
  * @code
  *
  * XC_DECLARE_COMPONENT {
  *    "MyComponentName",
  *    "a short description of my component",
  *    1,
  *    2,
  *    my_init,
  *    my_destroy,
  *    {
  *       XC_DECLARE_PROVIDED_PORT (
  *          "Application",             // Name of the provided port
  *          (xc_interface_t *) &myApp, // Pointer to exported switch
  *          sizeof (myApp),            // Size of the export switch
  *          NULL,                      // Port name
  *          NULL,                      // Reserved for future use, pass NULL
  *          my_app_register,           // register() for this port
  *          my_app_unregister          // unregister() for this port
  *       ),
  *       NULL                          // End the port array with a NULL entry
  *    }
  * }
  *
  * @endcode
  *
  */
#define XC_DECLARE_COMPONENT                              \
   struct __xc_component_decl__ XC_DECLARED_COMPONENT_SYM \
      __attribute__((visibility("default"))) =

/**
  * Declare an xCOM "provided interface" port. The macro should be used within
  * a component declaration (see XC_DECLARE_COMPONENT).
  *
  * @hideinitializer
  *
  * @param n name of the port
  * @param i pointer to the exported interface switch
  * @param sz size of exported interface switch
  * @param p name of the port
  * @param r reserved for future use, pass NULL
  * @param reg register() method for this port
  * @param unreg unregister() method for this port
  *
  */
#define XC_DECLARE_PROVIDED_PORT(n,i,sz,p,r,reg,unreg) \
   {                                                   \
      n,                 /* name */                    \
      i,                 /* interfacePtr */            \
      sz,                /* interfaceSize */           \
      NULL,              /* component */               \
      p,                 /* port */                    \
      r,                 /* reserved */                \
      XC_PORTF_PROVIDED, /* flags */                   \
      reg,               /* onRegister */              \
      unreg,             /* onUnRegister */            \
      NULL               /* importHandlePtr */         \
   }

/**
  * Declare an xCOM "required interface" port. The macro should be used within
  * a component declaration (see XC_DECLARE_COMPONENT).
  *
  * @hideinitializer
  *
  * @param name of the "required interface" port
  * @param i pointer to the imported interface declaration
  * @param h where to store the import handle
  * @param c name of the queried component or NULL
  * @param p name of the queried port or NULL
  * @param r reserved for future use, pass NULL
  * @param reg register() method for this port
  * @param unreg unregister() method for this port
  * @param f flags for this port (see XC_PORTF_ defines)
  *
  */
#define XC_DECLARE_REQUIRED_PORT(n,i,h,c,p,r,reg,unreg,f) \
   {                                                      \
      n,                     /* name */                   \
      i,                     /* interfacePtr */           \
      0,                     /* interfaceSize */          \
      c,                     /* component */              \
      p,                     /* port */                   \
      r,                     /* reserved */               \
      XC_PORTF_REQUIRED | f, /* flags */                  \
      reg,                   /* onRegister */             \
      unreg,                 /* onUnRegister */           \
      h                      /* importHandlePtr */        \
   }

/**
  * Header to be put in front of any declared xCOM interface.
  * For instance:
  *
  * @hideinitializer
  * @code
  *
  * typedef struct sample_interface {
  *    XC_INTERFACE;
  *    void (* my_sample_method) (void);
  * } sample_interface_t;
  *
  * @endcode
  *
  */
#define XC_INTERFACE xc_interface_t __xc_interface_decl__

/**
  * Initialize an xCOM interface declaration.
  *
  * @hideinitializer
  * @param name name of the interface
  * @param vmaj major version number of the interface
  * @param vmin minor version number of the interface
  *
  */
#define XC_INTERFACE_INIT(name,vmaj,vmin) { \
   name,                                    \
   vmaj,                                    \
   vmin                                     \
}

/**
  * Initialize the xCOM framework. This function may be called multiple times.
  *
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */ 
xc_result_t
xCOM_Init (
   void
)
XCOM_EXPORT;

/**
  * Free resources used by the xCOM framework. If xCOM_Init() had been called
  * multiple times, xCOM_Destroy() must be called as many times for resources
  * to be actually freed up.
  *
  */
void
xCOM_Destroy (
   void
)
XCOM_EXPORT;

/**
  * Load an xCOM component bundle.
  *
  * @param path path to the component bundle to be loaded.
  * @param flags load flags, see XC_LOADF_ defines.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_LoadComponentBundle (
   const char *path,
   unsigned int flags
)
XCOM_EXPORT;

/**
  * Return the path to the component bundle of a component.
  *
  * @param componentHandle handle of the component.
  * @return the path to the component bundle the component was loaded from
  *
  */
const char *
xCOM_GetComponentBundlePath (
   xc_handle_t componentHandle
)
XCOM_EXPORT;

/**
  * Set a user-specific value for the specified component. This should be used
  * by xCOM language bindings to attach a context object to the component.
  *
  * @param componentHandle handle of the component
  * @param user_data data to be stored in the component
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_SetSpecific (
   xc_handle_t componentHandle,
   void *user_data
)
XCOM_EXPORT;

/**
  * Get the user-specific value for the specified component. The value should have
  * been previously set with xCOM_SetSpecific or NULL will be returned.
  *
  * @param componentHandle handle of the component
  * @return the user-defined value or NULL on error (or if not set).
  *
  */
void *
xCOM_GetSpecific (
   xc_handle_t componentHandle
)
XCOM_EXPORT;

/**
  * Execute the xCOM framework event loop.
  *
  * @return XC_OK on success, an xCOM error otherwise.
  *
  */
xc_result_t
xCOM_Exec (
   void
)
XCOM_EXPORT;

/**
  * Make the xCOM framework terminate its event loop.
  *
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_Quit (
   void
)
XCOM_EXPORT;

/**
  * Create a query for an xCOM interface.
  *
  * @param componentHandle handle of the component placing the query.
  * @param interfaceName name of the queried interface.
  * @param interfaceMajorVersion major version of the interface
  * @param interfaceMinorVersion minor version of the interface
  * @param queriedComponentName non-NULL if the query is targetting a specific component. 
  * @param queriedPortName non-NULL to target a specific port.
  * @param queryFlags flags for this query, see XC_QUERYF_ defines.
  * @param queryHandlePtr where to store the created query handle.
  * @param matchCountPtr where to store the number of matches.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_QueryInterface (
   xc_handle_t componentHandle,
   const char *interfaceName,
   unsigned int interfaceMajorVersion,
   unsigned int interfaceMinorVersion,
   const char *queriedComponentName,
   const char *queriedPortName,
   unsigned int queryFlags,
   xc_handle_t *queryHandlePtr,
   unsigned int *matchCountPtr
)
XCOM_EXPORT;

/**
  * Create a query for the specified "required interface" port.
  *
  * @param componentHandle handle of the component issuing the query.
  * @param portName name of the "required interface" port.
  * @param queryFlags flags for this query, see XC_QUERYF_ defines.
  * @param queryHandlePtr where to store the created query handle.
  * @param matchCountPtr where to store the number of matches.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_QueryPort (
   xc_handle_t componentHandle,
   const char *portName,
   unsigned int queryFlags,
   xc_handle_t *queryHandlePtr,
   unsigned int *matchCountPtr
)
XCOM_EXPORT;

/**
  * Close a previously created query.
  *
  * @param queryHandle handle of the query to be freed.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_QueryFinish (
   xc_handle_t queryHandle
)
XCOM_EXPORT;

/**
  * Get the next matching import from a query. Note that while the returned
  * import exists, the connection to the server component wasn't established.
  *
  * @param queryHandle handle of the query.
  * @param importHandlePtr where to store the handle of the next matching import.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_QueryNext (
   xc_handle_t queryHandle,
   xc_handle_t *importHandlePtr
)
XCOM_EXPORT;

/**
  * Open an import returned by xCOM_QueryNext. This method will actually cause
  * the connection between the client and server component to be established. On
  * the server side, if a register method was declared in the provided port
  * declaration, it would have been called to let the server know that a component
  * is connecting to its service. The server then having the option to either
  * accept or deny the service request.
  *
  * @param importHandle handle of the import
  * @param interfacePtr where to store the switch pointer
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_Import (
   xc_handle_t importHandle,
   xc_interface_t **interfacePtr
)
XCOM_EXPORT;

/**
  * Set a user-specific value for the specified import and component. This
  * should be used by xCOM language bindings to attach an object to the
  * xCOM import either on the import or export side.
  *
  * @param componentHandle handle of the component (either server or client)
  * @param importHandle handle of the import
  * @param user_data data to be stored in the import
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_ImportSetSpecific (
   xc_handle_t componentHandle,
   xc_handle_t importHandle,
   void *user_data
)
XCOM_EXPORT;

/**
  * Get the user-specific value for the specified import and component. The
  * value should have been previously set with xCOM_ImportSetSpecific or
  * NULL will be returned.
  *
  * @param componentHandle handle of the component (either server or client)
  * @param importHandle handle of the import
  * @return the user-defined value or NULL on error (or if not set).
  *
  */
void *
xCOM_ImportGetSpecific (
   xc_handle_t componentHandle,
   xc_handle_t importHandle
)
XCOM_EXPORT;

/**
  * Close a previously opened import. This method is to be called from the client
  * component (would not be safe from the server as it cannot assume how client(s)
  * are implemented).
  *
  * @param importHandle handle of the import to be closed.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
xCOM_UnImport (
   xc_handle_t importHandle
)
XCOM_EXPORT;

/**
  * Get the interface switch for an opened interface.
  *
  * @param importHandle handle of the opened import.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_interface_t *
xCOM_GetSwitch (
   xc_handle_t importHandle
)
XCOM_EXPORT;

/**
  * Get the handle of the next sibling import. Imports created from the same
  * query are linked together. This is for instance the case for loadtime
  * multiple imports (ports having set the XC_PORTF_MULTIPLE flag). This
  * method provides the handle of the next import within the sibling chain.
  *
  * @param importHandle handle of the import
  * @return the handle of the sibling import or XC_INVALID_HANDLE
  *
  */
xc_handle_t
xCOM_GetNextImport (
   xc_handle_t importHandle
)
XCOM_EXPORT;
 
#ifdef __cplusplus
}
#endif

/**
  * @mainpage
  * @{
  * xCom is a basic component framework to break down middleware applications into smaller
  * reusable components. A component exposes its services via "provided interface" ports.
  * Dependencies are also expressed via "required interface" ports. The framework is then
  * having a database of all the components and the services they provide as well as their
  * dependencies. As components are implemented as shared library and as provides and
  * requires are known, the xCOM framework will load only needed components. Applications
  * may also issue xCOM queries to discover services at runtime and load or unload them
  * as needed. When xCOM is finding a component to be no longer used, it will unload
  * the component as well as all its dependencies that would be found unused and consequently
  * keep in the runtime environment only the code found needed.
  * @}
  */
#endif /* XCOM_H */

