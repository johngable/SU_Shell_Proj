
#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define list_entry(ptr, type, member) ({ \
   void *__mptr = (void *)(ptr);        \
   ((type *)(__mptr - offsetof(type, member)));  \
     })


struct list_head { 
    struct list_head *next;
    struct list_head *prev;
};

void list_add(struct list_head *new, struct list_head *head);
void list_add_tail(struct list_head *new, struct list_head *head);
void list_del(struct list_head *entry);
int list_empty(struct list_head *head);
void list_splice(struct list_head *list, struct list_head *head);
int getListLength(struct list_head *list);



#endif //End LIST_H