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

#ifndef INTERFACES_H
#define INTERFACES_H

#ifdef __cplusplus
extern "C" {
#endif

#define MATCHF_NONE         0
#define MATCHF_SAME_VERSION 1 /**< Match if versions are the same. */

/**
  * Check if two interfaces match, that is if the interface names match. If
  * a port was specified for both, it should also match. Major version
  * numbers shall always match. If called with the MATCHF_SAME_VERSION flag,
  * the minor version numbers shall also match, otherwise a greater minor
  * version number for interface1 is accepted.
  *
  * @param interface1Ptr pointer to the first xCOM interface switch.
  * @param port1 port for interface1
  * @param reserved1 will be used in the future, pass NULL
  * @param interface2Ptr pointer to the second xCOM interface switch.
  * @param port2 port for interface2
  * @param reserved2 will be used in the future, pass NULL
  * @param flags match flags
  *
  * @return true if the interfaces match, false otherwise.
  *
  */
bool
interfaces_match (
   const xc_interface_t *interface1Ptr,
   const char *port1,
   void *reserved1,
   const xc_interface_t *interface2Ptr,
   const char *port2,
   void *reserved2,
   int flags
);
   
#ifdef __cplusplus
}
#endif

#endif /* INTERFACES_H */

