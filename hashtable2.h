/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Statically sized hash table implementation
 * (C) 2012  Sasha Levin <levinsasha928@gmail.com>
 */

#ifndef _LINUX_HRBTREE_H
#define _LINUX_HRBTREE_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/hashtable.h>
#include <linux/rbtree.h>
#include <linux/hash.h>
#include <linux/rculist.h>

struct hrbtree_head{
	struct rb_root root;
}

/*all nodes that has same key will be stored in hlist data structure
 *as original hashtable does. Each node will store the head of the hlist and hnode.
 */

struct hrbtree_node{
	struct rb_node node;
	struct hrbtree_node* eq; //will be used to store data with same key.
	u32 key;
};

#define HRBTREE_INIT { .root=RB_ROOT }
#define INIT_HRBTREE(ptr) { ptr->root=RB_ROOT }
#define DEFINE_HRBTABLE(name, bits)						\
	struct hrbtree name[1 << (bits)] =					\
			{ [0 ... ((1 << (bits)) - 1)] = HRBTREE_INIT }

#define DECLARE_HRBTABLE(name, bits)                                   	\
	struct hrbtree name[1 << (bits)]

#define HRBTREE_SIZE(name) (ARRAY_SIZE(name))
#define HRBTREE_BITS(name) ilog2(HASH_SIZE(name))
#define HRBTREE_NODE {.eq=NULL}
/* Use hash_32 when possible to allow for fast 32bit hashing in 64bit kernels. */
#define hrbtree_min(val, bits)							\
	(sizeof(val) <= 4 ? hash_32(val, bits) : hash_long(val, bits))

static inline void __hrbtree_init(struct hrbtree *ht, unsigned int sz)
{
	unsigned int i;
	for (i = 0; i < sz; i++)
		INIT_HRBTREE(&ht[i]);
}

/**
 * hash_init - initialize a hash table
 * @hashtable: hashtable to be initialized
 *
 * Calculates the size of the hashtable from the given parameter, otherwise
 * same as hash_init_size.
 *
 * This has to be a macro since HASH_BITS() will not work on pointers since
 * it calculates the size during preprocessing.
 */
#define hrbtree_init(hashtable) __hrbtree_init(hashtable, HRBTREE_SIZE(hashtable))

/**
 * hash_add - add an object to a hashtable
 * @hashtable: hashtable to add to
 * @node: the &struct hlist_node of the object to be added
 * @key: the key of the object to be added
 */
 
static inline void hrbtree_add_head(struct hrbtree_node* n, u32 key, struct hrbtree_head* h)
{
	struct rb_node**   curr = &(h->root->rb_node);
	struct rb_node*  parent = NULL;
	struct hrbtree_node* new_node = NULL;
	u32 curr_key;
	while(*curr){
		curr_key = rb_entry(*curr, struct hrbtree_node, node)->key;
		if(curr_key < key){
			curr = &((*curr)->rb_right);
		}
		else if(curr_key > key){
			curr = &((*curr)->rb_left);
		}
		else{
			//item with same key exists. linked list will be used here.
			if(rb_entry(*curr, struct hrbtree_node, node)->eq == NULL){
				
			}
			curr = &(rb_entry(*curr, struct hrbtree_node, node)->eq->node);
			return;
		}
	}
	//item with same key does not exist. initialize hrbtree_node and expand rbtree.
	new_node = kmalloc(sizeof(struct hrbtree_node), GFP_KERNEL);
	if(!new_node){
		printk("Unable to malloc new node");
		return;
	}
	new_node->head = HLIST_HEAD_INIT;
	rb_link_node(&(new_node->rbnode), parent, curr);
	rb_insert_color(&(new_node->rbnode), h->root);
	return;
}
#define hrbtree_add(hashtable, node, key)						\
	hrbtree_add_head(node, key, &hashtable[hash_min(key, HASH_BITS(hashtable))])

static inline bool __hrbtree_empty(struct hrbtree *ht, unsigned int sz)
{
	unsigned int i;
	for (i = 0; i < sz; i++)
		if (!RB_EMPTY_NODE(&ht[i]->root))
			return false;
	return true;
}

/**
 * hash_empty - check whether a hashtable is empty
 * @hashtable: hashtable to check
 *
 * This has to be a macro since HASH_BITS() will not work on pointers since
 * it calculates the size during preprocessing.
 */
#define hrbtree_empty(hashtable) __hrbtree_empty(hashtable, HASH_SIZE(hashtable))

/**
 * hash_del - remove an object from a hashtable
 * @node: &struct hlist_node of the object to remove
 */
static inline void hrbtree_del(struct hlist_node *node)
{
	container_of(node, struct hrbtree_node, hnode)->head;
	hlist_del_init(node);
}


/**
 * hash_for_each - iterate over a hashtable
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hlist_node within the struct
 */
#define hrbtree_for_each(name, bkt, obj, member)				\
	for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < HASH_SIZE(name);\
			(bkt)++)\
		hlist_for_each_entry(obj, &name[bkt], member)

/**
 * hash_for_each_safe - iterate over a hashtable safe against removal of
 * hash entry
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @tmp: a &struct used for temporary storage
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hlist_node within the struct
 */
#define hash_for_each_safe(name, bkt, tmp, obj, member)			\
	for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < HASH_SIZE(name);\
			(bkt)++)\
		hlist_for_each_entry_safe(obj, tmp, &name[bkt], member)

/**
 * hash_for_each_possible - iterate over all possible objects hashing to the
 * same bucket
 * @name: hashtable to iterate
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hlist_node within the struct
 * @key: the key of the objects to iterate over
 */
#define hash_for_each_possible(name, obj, member, key)			\
	hlist_for_each_entry(obj, &name[hash_min(key, HASH_BITS(name))], member)

/**
 * hash_for_each_possible_safe - iterate over all possible objects hashing to the
 * same bucket safe against removals
 * @name: hashtable to iterate
 * @obj: the type * to use as a loop cursor for each entry
 * @tmp: a &struct used for temporary storage
 * @member: the name of the hlist_node within the struct
 * @key: the key of the objects to iterate over
 */
#define hash_for_each_possible_safe(name, obj, tmp, member, key)	\
	hlist_for_each_entry_safe(obj, tmp,\
		&name[hash_min(key, HASH_BITS(name))], member)


#endif
