#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "point.h"
#include "sortedPoints.h"

/*
 * Initialize data structures, returning pointer
 * to the object.
 */

SortedPoints *sp_init(SortedPoints *sp){

	sp->head = NULL;
	return sp;
}

/*
 * Allocate a new point and initialize it to x,y. Then
 * add that point to the SortedPoints list. Return
 * 1 on success and 0 on error (e.g., out of memory).
 */
int sp_addNewPoint(SortedPoints *sp, double x, double y)
{
	Point ORIGIN;
	ORIGIN.x = 0;
	ORIGIN.y = 0;
        
        // Local iterator node
	Node *curr;
	
	// Allocate memory for the new node
	Node *n;
	n = (Node *)malloc(sizeof(Node));
	if (!n) return 0;  // malloc() returns a NULL pointer if not enough memory.
        
        // Initializing the new node 'n'
	n->p.x = x;
	n->p.y = y;
	
// 	Inserting the new node in the correct place in the linked list
	
	// If the list is empty, add the point as the head.
	if (sp->head == NULL) {
		sp->head = n;
		return 1;
	}

	curr = sp->head;

	while (curr != NULL) {
	
		if (point_distance(&n->p, &ORIGIN) < point_distance(&curr->p, &ORIGIN)) {
			insertBefore(sp, n, curr);
			return 1;
		}
		if(point_distance(&n->p, &ORIGIN) == point_distance(&curr->p, &ORIGIN)) {
			if(n->p.x < curr->p.x) {
				insertBefore(sp, n, curr);
				return 1;
			}
			if(n->p.x == curr->p.x && n->p.y < curr->p.y) {
				insertBefore(sp, n, curr);
				return 1;
			}
		}
		
		curr = curr->next;
	}
	
	// If we reach this point, n is not less than any of the other nodes and
	// we add it to the end of the list
	
	curr = sp->head;
	while (curr->next != NULL) curr = curr->next;
	insertAfter(n, curr);
	return 1;
}

/*
  * Remove the first point from the sorted list,
  * storing its value in *ret. Returns 1 on success
  * and 0 on failure (empty list).
  */
int sp_removeFirst(SortedPoints *sp, Point *ret)
{
    Node *savehead;
    
    if (sp->head == NULL) return 0;
    
    savehead = sp->head;
    sp->head = sp->head->next;
    free(savehead);
    return 1;
}

/*
  * Remove the last point from the sorted list,
  * storing its value in *ret. Returns 1 on success
  * and 0 on failure (empty list).
  */
int sp_removeLast(SortedPoints *sp, Point *ret)
{
	Node *curr;
	curr = sp->head;
	if (!curr) return 0;
	while (curr->next != NULL) curr = curr->next;
	ret = &curr->p;
	free(curr);
	return 1;
}



/*
  * Remove the point that appears in position
  * <index> on the sorted list, storing its 
  * value in *ret. Returns 1 on success
  * and 0 on failure (too short list).
  *
  * The first item on the list is at index 0.
  */
int sp_removeIndex(SortedPoints *sp, int index, Point *ret)
{
	int i;
	Node *curr;
	curr = sp->head;
	for (i = 0; i<index; i++) {
		if (!curr) return 0;
		curr = curr->next;
	}
	if (!curr) return 0;
	ret = &curr->p;
	free(curr);
	return 1;
}


/*
  * Delete any duplicate records. E.g., if
  * two points on the list have *identical*
  * x and y values, then delete one of them.
  * Return the number of records deleted.
  */
int sp_deleteDuplicates(SortedPoints *sp)
{
	int returnValue;
	returnValue = 0;
	Node *curr, *curr2;
	curr = sp->head;
	
	while (curr != NULL) {
		curr2 = curr->next;
		while (curr2 != NULL) {
			if (curr->p.x == curr2->p.x && curr->p.y == curr2->p.y) {
				deleteNode(sp, curr2);
				free(curr2);
				returnValue++;
			}
		}
	}
	
	return returnValue;
}

void deleteNode(SortedPoints *sp, Node *del) {

	Node *prev;
	prev = sp->head;
	while (prev->next != del && prev != NULL) prev = prev->next;
	
	if (prev == sp->head) {
		sp->head = prev->next;
	}
	
	prev->next = del->next;

}

void insertAfter(Node *new, Node *ref) {

	new->next = ref->next;
	ref->next = new;

}

int insertBefore(SortedPoints *sp, Node *new, Node *ref) {

	Node *prev;
	prev = sp->head;
        
        // If the new node is to be added at the head of the list
        if (ref == sp->head) {
            new->next = ref;
            sp->head = new;
            return 0;
        }
        
	while (prev->next != ref && prev != NULL) {
		prev = prev->next;
	}
	if (prev == NULL) return 1;
	
	new->next = ref;
	prev->next = new;
	
	return 0;
	

}

/*
 * Prints the specified SortedPoints list line by line
 * followed by a horizontal break.  For debugging purposes.
 */

void printList(SortedPoints *sp) {
    
    Node *curr;
    curr = sp->head;
    
    while (curr != NULL) {
        printf("%.1f, %.1f\n", curr->p.x, curr->p.y);
        curr = curr->next;
    }
    
    printf("-----------------\n");
    
}