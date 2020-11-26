#include <stdio.h>
#include <stdlib.h>

typedef struct list_s {

    struct list_s *next; /* NULL for the last item in a list */
    int data;

}	list_t;

/* Counts the number of items in a list.	 */

int count_list_items(const list_t *head) {
	if (head->next) {
		return count_list_items(head->next) + 1;	// recursive call, stack overflow, what if NULL passed
	} else {
		return 1;
	}
}

int count_list_items2(const list_t *head)
{
	list_t *p;
	int n;

	for (n=0, p=head; p; n++)
		p = p->next;

	return n;
}

/* Inserts a new list item after the one specified as the argument.	 */

void insert_next_to_list(list_t *item, int data) {
	(item->next = malloc(sizeof(list_t)))->next = item->next;	// what if malloc fails; item->next overwritten
	item->next->data = data;
}

int insert_next_to_list2(list_t *item, int data)
{
	list_t *p = malloc(sizeof(list_t));

	if (p) {
		p->data = data;
		p->next = item->next;
		item->next = p;
	}

	return !p;
}


/* Removes an item following the one specificed as the argument.	 */
void remove_next_from_list(list_t *item) {
	 if (item->next) {
		free(item->next);
        item->next = item->next->next;			// item->next->next in deallocated memory 
     }
}

void remove_next_from_list2(list_t *item) 
{
	list_t *p;
	if (item) {
		p = item->next;
		if (p) {
			item->next = p->next;
			free(p);
		}
	}
}


/* Returns item data as text.	 */
char *item_data(const list_t *list)
{
	char buf[12];

	sprintf(buf, "%d", list->data);	// why size is assumed to be <12
	return buf;						// buf[12] on stack, destroyed on return
}

void item_data2(const list_t *list, char *txtbuf, int tbsize)
{
	if (list) {
		snprintf(txtbuf, tbsize, "%d", list->data);
	}
}
