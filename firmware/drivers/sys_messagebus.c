
/*
    openchronos system messagebus implementation

         Copyright (C) 2012 Angelo Arrifano <miknix@gmail.com>

                  http://www.openchronos-ng.sourceforge.net

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sys_messagebus.h"

void sys_messagebus_register(void (*callback) (enum sys_message),
                             enum sys_message listens)
{
    struct sys_messagebus **p = &messagebus;

    while (*p) {
        p = &(*p)->next;
    }

    *p = malloc(sizeof(struct sys_messagebus));
    (*p)->next = NULL;
    (*p)->fn = callback;
    (*p)->listens = listens;
}

void sys_messagebus_unregister(void (*callback) (enum sys_message))
{
    struct sys_messagebus *p = messagebus, *pp = NULL;

    while (p) {
        if (p->fn == callback) {
            if (!pp)
                messagebus = p->next;
            else
                pp->next = p->next;

            free(p);
        }

        pp = p;
        p = p->next;
    }
}
