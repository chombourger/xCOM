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

#define  TRACE_CLASS_DEFAULT HANDLEDIR
#include "internal.h"

/* Page entry manipulation macros */
#define  MAKE_PRESENT(p) ((void **)(((unsigned long)(p)) | 1))
#define  GET_PTR(p) ((void **)(((unsigned long)(p)) & ~1))
#define  IS_PRESENT(p) (((unsigned long)(p)) & 1)

/** Structure describing a page of handles */
struct HandlePage {
   /** Node into list of pages with free entries */
   xc_clist_t node;
   /** Pointer to first free entry in the page */
   void **first;
   /** Number of entries in use in the page */
   short usage;
   /** Maximum entries in the page */
   short entries;
   /** Entry number in directory */
   unsigned int dirent;
   /** Entries. Actual number depends on pool size */
   void *entry [3];
};

/** Structure describing a directory of pages of handles */
struct HandleDir {
   /** List of pages with free entries */
   xc_clist_t pages;
   /** Pointer to first free entry in the dir */
   void **first;
   /** Entries. Actual number depends on pool size */
   void *entry [3];
};

/* Number of entries in a handles pool */

#define  NUM_ENTRIES \
   (((HANDLE_DIR_POOL_SIZE - sizeof (struct HandlePage)) / sizeof (void**)) + 3)

#define  NUM_PAGES \
   (((HANDLE_DIR_POOL_SIZE - sizeof (struct HandleDir)) / sizeof (void**)) + 3)

#define  PAGE_DIV (HANDLE_DIR_POOL_SIZE / sizeof (void **))

/** Initialize entries in a page */
static inline void
init_page (
   void **hPagePtr,
   int    stepping
) {
   void **hPageEndPtr;

   TRACE3 (("called with hPagePtr=%p, stepping=%d", hPagePtr, stepping));
   
   hPageEndPtr = hPagePtr + (NUM_ENTRIES - (NUM_ENTRIES%stepping)) - stepping;

   do {
      void **hNextPagePtr = hPagePtr + stepping;
      
      *hPagePtr = hNextPagePtr;
      hPagePtr = hNextPagePtr;
   } while (hPagePtr != hPageEndPtr);

   *hPagePtr = NULL;
   TRACE3 (("exiting"));
}

/* Initialize entries in a directory */

static inline void
init_dir (
   void **hPagePtr
) {
   void **hPageEndPtr;
 
   TRACE3 (("called with hPagePtr=%p", hPagePtr));
 
   hPageEndPtr = hPagePtr + NUM_PAGES - 1;

   do {
      void **hNextPagePtr = hPagePtr + 1;
      
      *hPagePtr = hNextPagePtr;
      hPagePtr = hNextPagePtr;
   } while (hPagePtr != hPageEndPtr);

   *hPagePtr = NULL;
   TRACE3 (("exiting"));
}

xc_handle_t
handle_dir_push (
   void *hDirPtr, 
   void *objPtr
) {
   struct HandleDir   *curHDirPtr = hDirPtr;
   struct HandlePage  *hPagePtr;
   struct HandlePage  *fPagePtr = NULL;
   void              **entry;
   xc_handle_t         handle;

   TRACE3 (("called with hDirPtr=%p, objPtr=%p", hDirPtr, objPtr));

   if (curHDirPtr->pages.next == (xc_clist_t *) &curHDirPtr->pages) {
      entry = curHDirPtr->first;
      if (!entry) {
         /* No more directory entries */
         return XC_INVALID_HANDLE;
      }
     
      hPagePtr = malloc (HANDLE_DIR_POOL_SIZE);
      if (!hPagePtr) return XC_INVALID_HANDLE;
      
      if (curHDirPtr->pages.next == (xc_clist_t *) &curHDirPtr->pages) {
         init_page ((void **) &hPagePtr->entry[0], 1);
         hPagePtr->entries = NUM_ENTRIES;

         hPagePtr->dirent = entry - &curHDirPtr->entry[0];
         hPagePtr->usage = 0;
         hPagePtr->first = &hPagePtr->entry[0];
         XC_CLIST_ADDTAIL (&curHDirPtr->pages, hPagePtr);
      
         curHDirPtr->first = (void **) *entry;
         *entry = (void *) MAKE_PRESENT (hPagePtr);
      }
      else {
         /* A page was made available in the meantime. Dump this one. */
         fPagePtr = hPagePtr;
      }
   }
   
   hPagePtr = (struct HandlePage *) curHDirPtr->pages.next;
   entry = hPagePtr->first;
   hPagePtr->first = *entry;
   
   entry[0] = MAKE_PRESENT (objPtr);
   handle = (entry - &hPagePtr->entry[0]);
   handle += (hPagePtr->dirent * PAGE_DIV) + 1;
   
   hPagePtr->usage++;
   if (!hPagePtr->first) XC_CLIST_REMOVE (hPagePtr);

   if (fPagePtr) free (fPagePtr);

   TRACE3 (("exiting with result=%u", handle));
   return (xc_handle_t) handle;
}

void
handle_dir_remove (
   void        *hDirPtr, 
   xc_handle_t  handle
) {
   struct HandleDir   *curHDirPtr = hDirPtr;
   struct HandlePage  *hPagePtr;
   struct HandlePage  *fPagePtr = NULL;
   unsigned int        didx;
   unsigned int        pidx;
   void              **entry;

   TRACE3 (("called with hDirPtr=%p, handle=%u", hDirPtr, handle));

   handle --;
   didx = handle / PAGE_DIV;

   if (didx < NUM_PAGES) {
      hPagePtr = curHDirPtr->entry[didx];

      if (IS_PRESENT (hPagePtr)) {
         hPagePtr = (struct HandlePage *) GET_PTR (hPagePtr);

         pidx = handle % PAGE_DIV;
         if (hPagePtr->entries != NUM_ENTRIES) pidx <<= 1;

         if (pidx < NUM_ENTRIES) {
            entry = (void **) &hPagePtr->entry[pidx];

            if (IS_PRESENT (*entry)) {
               *entry = (void *) hPagePtr->first;
               hPagePtr->first = entry;
               hPagePtr->usage--;

               if (!hPagePtr->usage) {
                  curHDirPtr->entry[didx] = curHDirPtr->first;
                  curHDirPtr->first = &curHDirPtr->entry[didx];
   
                  XC_CLIST_REMOVE (hPagePtr);
                  fPagePtr = hPagePtr;
               }
               else if (hPagePtr->usage == (hPagePtr->entries - 1)) {
                  XC_CLIST_ADDHEAD (&curHDirPtr->pages, hPagePtr);
               }
            }
         }
      }
      if (fPagePtr) free (fPagePtr);
   }
   TRACE3 (("exiting"));
}

void *
handle_dir_new (
   void
) {
   
   struct HandleDir *curHDirPtr;

   TRACE3 (("called"));

   curHDirPtr = malloc (HANDLE_DIR_POOL_SIZE);
   if (curHDirPtr != NULL) {
      init_dir ((void **) &curHDirPtr->entry[0]);
      curHDirPtr->first = &curHDirPtr->entry[0];
      XC_CLIST_INIT (&curHDirPtr->pages);
   }

   TRACE3 (("exiting with result=%p", curHDirPtr));     
   return (void *) curHDirPtr;
}

void
handle_dir_destroy (
   void *hDirPtr
) {

   TRACE3 (("called with hDirPtr=%p", hDirPtr));
 
   if (hDirPtr != NULL) {
      struct HandleDir *curHDirPtr = (struct HandleDir *) hDirPtr;
      void **entry;

      entry = &curHDirPtr->entry[0];

      while (entry < (void **)((char *)curHDirPtr->entry + (NUM_PAGES * sizeof (void *)))) {
         void **hPagePtr = (void **) *entry++;
         if (IS_PRESENT (hPagePtr)) free (GET_PTR(hPagePtr));
      }
      free (hDirPtr);
   }
   TRACE3 (("exiting"));
}

void *
handle_dir_get (
   void        *hDirPtr, 
   xc_handle_t  handle
) {
   struct HandleDir *curHDirPtr = (struct HandleDir *) hDirPtr;
   struct HandlePage *hPagePtr;
   unsigned int didx, pidx;
   void *obj, *ptr = NULL;

   TRACE3 (("called with hDirPtr=%p, handle=%u", hDirPtr, handle));

   handle --;
   didx = handle / PAGE_DIV;

   if (didx < NUM_PAGES) {
      hPagePtr = curHDirPtr->entry[didx];

      if (IS_PRESENT (hPagePtr)) {
         hPagePtr = (struct HandlePage *) GET_PTR (hPagePtr);

         pidx = handle % PAGE_DIV;
         if (pidx < NUM_ENTRIES) {
            obj = hPagePtr->entry[pidx];

            if (IS_PRESENT(obj)) {
               ptr = GET_PTR (obj);
            }
         }
      }
   }

   TRACE3 (("exiting with result=%p", ptr));
   return ptr;
}

