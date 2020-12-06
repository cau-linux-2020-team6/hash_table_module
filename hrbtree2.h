/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Statically sized hash table implementation
 * (C) 2012  Sasha Levin <levinsasha928@gmail.com>
 */

#ifndef _LINUX_HRTABLE_H
#define _LINUX_HRTABLE_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/hash.h>
#include <linux/rbtree.h>
#include <linux/rculist.h>

struct hrb_head{
	struct rb_node node;
	struct list_head eqhead;
	u32 key;
}

#define DEFINE_HASHTABLE(name, bits)						\
	struct hrb_head name[1 << (bits)]

#define DECLARE_HASHTABLE(name, bits)                                   	\
	DEFINE_HASHTABLE(name, bits)


/**
 * hash_add - add an object to a hashtable
 * @hashtable: hashtable to add to
 * @node: the &struct hlist_node of the object to be added
 * @key: the key of the object to be added
 */
static inline void hrb_add_head(struct hrb_head *n, u32 key, struct hrb_head *h)
{
	struct rb_node**   curr = &(h->node->rb_node);
	struct rb_node*  parent = NULL;
	struct hrb_head* new_node = NULL;
	u32 curr_key;
	while(*curr){
		curr_key = rb_entry(*curr, struct hrb_head, node)->key;
		if(curr_key < key){
			curr = &((*curr)->rb_right);
		}
		else if(curr_key > key){
			curr = &((*curr)->rb_left);
		}
		else{
			//item with same key exists. linked list will be used here.
			while(!rb_entry(*curr, struct hrb_head, node)->eq == NULL){
				parent = *curr;
				curr = &(rb_entry(*curr, struct hrb_head, node)->eq->node->rb_node);
			}
			new_node = kmalloc(sizeof(struct hrbtree_node), GFP_KERNEL);
			if(!new_node){
				printk("Unable to malloc new node");
				return;
			}
			new_node->key = key;
			list_add(new_node->eqhead, rb_entry(parent, struct hrb_head, node));
			return;
		}
	}
	//item with same key does not exist. initialize hrbtree_node and expand rbtree.
	new_node = kmalloc(sizeof(struct hrbtree_node), GFP_KERNEL);
	if(!new_node){
		printk("Unable to malloc new node");
		return;
	}
	LIST_HEAD_INIT(new_node->eqhead);
	new_node->key = key
	rb_link_node(&(new_node->rbnode), parent, curr);
	rb_insert_color(&(new_node->rbnode), h->root);
	return;
}


#define hrb_add(hashtable, node, key)						\
	hrb_add_head(node, &hashtable[hash_min(key, HASH_BITS(hashtable))])










/**
 * hash_del - remove an object from a hashtable
 * @node: &struct hlist_node of the object to remove
 */
static inline void hash_del(struct hlist_node *node)
{
	hlist_del_init(node);
}

/*
 * hash_for_each - iterate over a hashtable
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the hlist_node within the struct
 */
#define hash_for_each(name, bkt, obj, member)				\
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
