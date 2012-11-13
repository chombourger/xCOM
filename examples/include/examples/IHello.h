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

#ifndef EXAMPLES_IHELLO_H
#define EXAMPLES_IHELLO_H

#include <xCOM.h>

#define EXAMPLES_IHELLO_NAME          "examples.IHello"
#define EXAMPLES_IHELLO_VERSION_MAJOR 1
#define EXAMPLES_IHELLO_VERSION_MINOR 0

typedef struct examples_IHello {
   XC_INTERFACE;
   xc_result_t (* say) (const char *greeting);
} examples_ihello_t;

#endif /* EXAMPLES_IHELLO_H */

