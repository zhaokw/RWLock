#ifndef HTABLE_H
#define HTABLE_H

#include "rwlock.h"

#define BIG_PRIME 1560007

//node is the type of a linked list node type. Each hash table entry corresponds to a linked list containing key/value tuples that are hashed to the same slot.
typedef struct node {
	int hashcode;
	char *key;
	void *val;
	struct node *next;
      
}node;

typedef struct {
	int allow_resize;
	node **store; //to contain an array of linked list heads
	int size; //size of the array
        pthread_mutex_t* mutex;//Mutex to protect each slot array
  rwl* rwl;
}htable;

#endif
