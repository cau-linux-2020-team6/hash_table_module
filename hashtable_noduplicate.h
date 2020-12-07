/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Statically sized hash table implementation
 * (C) 2012  Sasha Levin <levinsasha928@gmail.com>
 */

#ifndef _LINUX_HASHRBTREE_H
#define _LINUX_HASHRBTREE_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/hash.h>
#include <linux/rbtree.h>
#include <linux/rculist.h>

struct hrb_node{
	struct rb_node node;
	u32 key;
};


void __hashrbtree_init(struct rb_root *ht, unsigned int sz)
{
	unsigned int i;

	for (i = 0; i < sz; i++)
		ht[i] = RB_ROOT;
}

int rb_insert(struct rb_root* root, struct hrb_node* hrb)
{
	struct rb_node** curr = &(root->rb_node), *parent = NULL;
	while(*curr){
		parent = *curr;
		if(rb_entry(*curr, struct hrb_node, node)->key < hrb->key){
			curr = &((*curr)->rb_right);
		}
		else if(rb_entry(*curr, struct hrb_node, node)->key > hrb->key){
			curr = &((*curr)->rb_left);
		}
		else return -1;
	}
	rb_link_node(&(hrb->node), parent, curr);
	rb_insert_color(&(hrb->node), root);
	return 0;
}
/**
 * hashrbtree_add - add an object to a hashtable
 * @hashtable: hashtable to add to
 * @node: the &struct hlist_node of the object to be added
 * @key: the key of the object to be added
 */
void hashrbtree_add(struct rb_root* hashtable, struct hrb_node* node, int hash_bits)
{
	rb_insert(&hashtable[hash_min(node->key, hash_bits)], node);
}

void rb_delete_tree(struct rb_node* root_node, struct rb_root* root)
{
	if(root_node->rb_left){
		rb_delete_tree(root_node->rb_left, root);
	}
	if(root_node->rb_right){
		rb_delete_tree(root_node->rb_right, root);
	}
	rb_erase(root_node, root);
}

static inline void hashrbtree_del(struct rb_root *node)
{
	rb_delete_tree(node->rb_node, node);
}

struct rb_node* rb_search(u32 key, struct rb_node* root)
{
	struct rb_node* curr = root;
	if(rb_entry(curr, struct hrb_node, node)->key == key){
		return curr;
	}
	else if(rb_entry(curr, struct hrb_node, node)->key < key){
		if(curr->rb_left) return rb_search(key, rb_prev(curr));
		else return NULL;
	}
	else{
		if(curr->rb_right) return rb_search(key, rb_next(curr));
		else return NULL;
	}

}

#define bucket_for(name, key) \
	&name[hash_min(key, HASH_BITS(name))]
/**
 * hash_for_each - iterate over a hashtable
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @iter: the type * to use as a loop cursor for each entry
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hlist_node within the struct
 */
#define hash_rbtree_for_each(name, bkt, iter)				\
	for ((bkt) = 0;(bkt) < HASH_SIZE(name);(bkt)++)\
		for((iter)=rb_first(&(name)[(bkt)]); (iter)!=NULL;(iter)=rb_next((iter)))


/**
 * hash_for_each_possible - iterate over all possible objects hashing to the
 * same bucket
 * @name: hashtable to iterate
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hlist_node within the struct
 * @key: the key of the objects to iterate over
 */
#define hash_rbtree_possible(name, key) \
	rb_search((key), name[hash_min(key, HASH_BITS(name))].rb_node)

#define hash_rbtree_for_each_possible(name, rptr, key) \
	for((rptr)=rb_first(bucket_for((name), (key)));(rptr)!=NULL;(rptr)=rb_next((rptr)))

#define DEFINE_HASHRBTREE(name, bits)						\
	struct rb_root name[1 << (bits)] =					\
			{ [0 ... ((1 << (bits)) - 1)] = RB_ROOT }

#define DECLARE_HASHRBTABLE(name, bits)                                   	\
	struct rb_root name[1 << (bits)]

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
#define hashrbtree_init(hashtable) __hashrbtree_init(hashtable, HASH_SIZE(hashtable))



#endif
