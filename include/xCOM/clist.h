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

#ifndef XCOM_CLIST_H
# define XCOM_CLIST_H

typedef struct xc_clist {
   struct xc_clist *next;
   struct xc_clist *prev;
} xc_clist_t;

# define XC_CLIST_INIT(l) do {	\
   (l)->next = (l)->prev = (l);	\
} while (0)

# define XC_CLIST_HEAD(l)  ((void *) (((xc_clist_t *)(l))->next))
# define XC_CLIST_TAIL(l)  (((xc_clist_t *)(l))->prev)
# define XC_CLIST_EMPTY(l) (((xc_clist_t *)(l))->next == (l))
# define XC_CLIST_END(l,n) (((xc_clist_t *)(l)) == ((xc_clist_t *)(n)))
# define XC_CLIST_NEXT(n)  ((void *) (((xc_clist_t *)(n))->next))
# define XC_CLIST_PREV(n)  (((xc_clist_t *)(n))->prev)

#define XC_CLIST_ADDHEAD(l,n) do {		\
   xc_clist_t *__n = (xc_clist_t *) (n);	\
   xc_clist_t *__t = (l)->next;			\
   						\
   __n->next = __t;				\
   __n->prev = (l);				\
   						\
   (l)->next = __n;				\
   __t->prev = __n;				\
} while (0)

# define XC_CLIST_ADDTAIL(l,n) do {		\
   xc_clist_t *__n = (xc_clist_t *) (n);	\
   xc_clist_t *__t = (l)->prev;			\
   						\
   __n->next = (l);				\
   __n->prev = __t;				\
   						\
   (l)->prev = __n;				\
   __t->next = __n;				\
} while (0)

# define XC_CLIST_REMOVE(n) do {		\
   xc_clist_t *__n = (xc_clist_t *) (n);	\
   xc_clist_t *__t = __n->prev;			\
   						\
   __n = __n->next;				\
   __t->next = __n;				\
   __n->prev = __t;				\
} while (0)

#endif /* XCOM_CLIST_H */

