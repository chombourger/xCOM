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

#ifndef COMPONENT_H
#define COMPONENT_H

#ifdef __cplusplus
extern "C" {
#endif

/** Component states. */
typedef enum {
   COMP_STATE_NULL,       /**< Initial state, component unusable. */
   COMP_STATE_LOADED,     /**< Component loaded but not yet initialized. */
   COMP_STATE_INITIALIZED /**< Component loaded and initialized. */
} component_state_t;

/**
  * Structure describing a component bundle.
  *
  */
struct Bundle {
   xc_clist_t node;       /**< List node for linking known component bundles. */
   char *path;            /**< Path to the bundle. */
   xc_clist_t components; /**< List of components in this bundle. */
   int dirfd;             /**< Handle of the bundle directory. */
};

/**
  * Structure describing an xCOM component.
  *
  */
struct Component {
   xc_clist_t node;         /**< List node for linking components of a bundle. */
   char *name;              /**< Name of this component. */
   char *descr;             /**< Component description. */
   unsigned int vmajor;     /**< Major version number. */
   unsigned int vminor;     /**< Minor version number. */
   bundle_t *bundlePtr;     /**< Pointer to the parent component bundle. */
   component_state_t state; /**< Component state. */
   xc_handle_t id;          /**< Component Runtime ID. */
   void *handle;            /**< Loaded library handle. */
   char *path;              /**< Path to the component shared library. */
   unsigned int references; /**< References to this component. */
   xc_clist_t imports;      /**< Imports this component is servicing. */
   unsigned int portsCount; /**< Total number of ports provided/required by this component. */
   port_t **ports;          /**< Ports of this component. */
   pthread_mutex_t lock;
   xc_result_t (* init) (xc_handle_t);
   xc_result_t (* destroy) (xc_handle_t);
};

/**
  * Initialize the xCOM component sub-module.
  *
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
component_module_init (
   void
);

/**
  * Destroy the xCOM component sub-module.
  *
  */
void
component_module_destroy (
   void
);

/**
  * Create a new component bundle.
  *
  * @param path directory where the component bundle can be found
  *
  * @return a pointer to the created component bundle.
  *
  */
bundle_t *
bundle_new (
   const char *path
);

/**
  * Free a component bundle.
  *
  * @param bundlePtr pointer to the component bundle.
  *
  */
void
bundle_free (
   bundle_t *bundlePtr
);

/**
  * Load components from a component bundle.
  *
  * @param bundlePtr pointer to the component bundle being loaded.
  * @param flags load flags, see XC_LOADF_ defines.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
bundle_load_components (
   bundle_t *bundlePtr,
   unsigned int flags
);

/**
  * De-initialize components that were loaded in a bundle.
  *
  * @param bundlePtr pointer to the component bundle.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
bundle_deinit_components (
   bundle_t *bundlePtr
);

/**
  * Get a reference to a component given its handle.
  *
  * @param componentHandle handle of the requested component
  * @return a pointer to the component on success, NULL otherwise.
  *
  */
component_t *
component_ref (
   xc_handle_t componentHandle
);

/**
  * Release a reference to a component.
  *
  * @param componentPtr pointer to the component.
  *
  */
void
component_unref (
   component_t *componentPtr
);

/**
  * Create a new component inside the specified bundle. Note, release
  * the component with component_unref() it is no longer needed. This
  * will cause its reference counter to be adjusted and the component
  * code unloaded when it reaches zero.
  *
  * @param bundlePtr pointer to the parent bundle.
  * @return a pointer to the created component on success, or NULL.
  *
  */
component_t *
component_new (
   bundle_t *bundlePtr
);

/**
  * Free a previously allocated component. One shall make sure there
  * are no references to it prior calling.
  *
  * @param componentPtr pointer to the component to be freed.
  *
  */ 
void
component_free (
   component_t *componentPtr
);

/**
  * Free the ports exposed by a component. Ports for "provided interfaces"
  * are also cleared from the providers hash.
  *
  * @param componentPtr pointer to the component for which ports shall be freed
  *
  */
void
component_free_ports (
   component_t *componentPtr
);

/**
  * Attach an import to a component. This is used for tracking imports opened
  * by a component so they get automatically closed when the component is no
  * longer used (i.e., none of its "provided interfaces" are imported.
  *
  * @param componentPtr pointer to the component that did the import
  * @param importPtr pointer to the import
  *
  */ 
void
component_add_import (
   component_t *componentPtr,
   import_t *importPtr
);

/**
  * Remove an import from a component.
  *
  * @param componentPtr pointer to the component
  * @param importPtr pointer to the import to be removed
  *
  */
void
component_remove_import (
   component_t *componentPtr,
   import_t *importPtr
);

/**
  * Get the handle of the special <anonymous> component. Queries/Imports from
  * non xCOM components get assigned to this special component so that from
  * an xCOM perspective, we do not have to implement special logic for such
  * imports.
  *
  * @return a handle to the special anonymous component.
  *
  */
xc_handle_t
component_get_anonymous (
   void
);

/**
  * Process loadtime imports of a component. This is a merely a query/import
  * done by the framework automatically just before the init method of the
  * component gets called. Similarly, such imports get closed automatically
  * right after the destroy method has been called.
  *
  * @param componentPtr pointer to the component being loaded
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
component_loadtime_imports (
   component_t *componentPtr
);

/**
  * Process a specific loadtime import of a component. This is called by
  * component_loadtime_imports for every port having the XC_PORTF_LOADTIME
  * flag.
  *
  * @param componentPtr pointer to the component being loaded
  * @return XC_OK on success, an xCOM error otherwise.
  *
  */
xc_result_t
component_loadtime_import (
   component_t *componentPtr,
   port_t *portPtr
);

/**
  * Close all the imports of a component. This method is being called when a component
  * is being unloaded.
  *
  * @param componentPtr pointer to the component being unloaded.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
component_close_imports (
   component_t *componentPtr
);

/**
  * Call the component init method if the component wasn't initialized yet. xCOM
  * will call this method shortly after a successful open of an import. This method
  * will check the current state of the component and will, if necessary, complete
  * its initialization (e.g., loading of loadtime imports).
  *
  * @param componentPtr pointer to the component to be initialized.
  * @return XC_OK on success, an xCOM error otherwise.
  *
  */
xc_result_t
component_init (
   component_t *componentPtr
);

/**
  * Call the component destroy method when the component is being unloaded.
  *
  * @param pointer to the component being unloaded
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
component_destroy (
   component_t *componentPtr
);

/**
  * Call the register method of a component's port for a "provided interface".
  * This method is being called whenever a component tries to import an
  * interface of a component. This allows the remote component to be notified
  * that someones is connecting to it and to either accept or reject the
  * request.
  *
  * @param componentPtr pointer to the component
  * @param portPtr pointer to the port of the "provided interface" being imported
  * @param importHandle handle of the import to be passed to register.
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
component_register (
   component_t *componentPtr,
   port_t *portPtr,
   xc_handle_t importHandle
);

/**
  * Call the unregister method of a component's port for a "provided interface".
  * This method is being called whenever a previously opened import is about to
  * be close. This allows the server component to clean-up things it would have
  * created for this import specifically.
  *
  * @param componentPtr pointer to the component
  * @param portPtr pointer to the port of the "provided interface" being closed
  * @param importHandle handle of the import to be passed to unregister
  * @return XC_OK on success, an xCOM error code otherwise.
  *
  */
xc_result_t
component_unregister (
   component_t *componentPtr,
   port_t *portPtr,
   xc_handle_t importHandle
);

/**
  * Set the name of a component.
  *
  * @param componentPtr pointer to the component
  * @param name name of the component or NULL to unset.
  * @return XC_OK on success, an xCOM error otherwise.
  *
  */
xc_result_t
component_set_name (
   component_t *componentPtr,
   const char *name
);

/**
  * Set the short description of a component.
  *
  * @param componentPtr pointer to the component
  * @param descr short description of the component or NULL to unset
  * @return XC_OK on success, an xCOM error otherwise.
  *
  */
xc_result_t
component_set_descr (
   component_t *componentPtr,
   const char *descr
);

/**
  * Get the port of a component given its name. Note this call will
  * lock the component until component_unref_port gets called. Also
  * note that this method will always return the first match should
  * the component have ports with the same name.
  *
  * @param componentPtr pointer to the component
  * @param name of the port to be looked up.
  * @return a pointer to the requested port on success, NULL otherwise.
  *
  */
port_t *
component_ref_port (
   component_t *componentPtr,
   const char *name
);

/**
  * Release reference to a previously acquired port reference (via
  * component_ref_port).
  *
  * @param componentPtr pointer to the component
  * @param portPort pointer to the port no longer being used
  *
  */
void
component_unref_port (
   component_t *componentPtr,
   port_t *portPtr
);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENT_H */

