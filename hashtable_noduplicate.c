#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include "hashtable_noduplicate.h"
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/ktime.h>
 
#define MY_HASH_BITS 2

//this file contains the example code for using our reconstructed hashtable,
//which may be faster if the collision occurs too much.
int num_to_test[1] = {1000};//, 10000, 100000, 1000000};

struct my_node {
	int value;
	struct hrb_node hnode;
};

void hash_example(void)
{
	int i, j, k;
	int bkt;
	struct rb_node* rptr;
	struct my_node* nptr;
	DEFINE_HASHRBTREE(my_hash, MY_HASH_BITS);
	for(i=0;i<sizeof(num_to_test)/sizeof(int);i++){
	
		hashrbtree_init(my_hash);
			
		//insert
		printk(KERN_INFO "begin insertion test, size=%d\n", num_to_test[i]);
		for(j=0;j<num_to_test[i];j++){
			for(k=0;k<2;k++){
				struct my_node *new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
				new->value = j * 10 + k;
				new->hnode.key = j;//intended collision
				hashrbtree_add(my_hash, &new->hnode, MY_HASH_BITS);
			}
		}
		printk(KERN_INFO "end insertion test\n");
		printk(KERN_INFO "begin search test\n");
		printk(KERN_INFO "\thash_for_each test\n");
		//search
		hash_rbtree_for_each(my_hash, bkt, rptr){
			//iterate every node in hashtable my_hash, and store the pointer of structure rb_node to the node onto nptr.
			//bkt means the hashed value.
			nptr = rb_entry(rptr, struct my_node, hnode.node);
			printk(KERN_INFO "value=%d, key=%d is in bucket %d\n", nptr->value, nptr->hnode.key, bkt);
		}
		printk(KERN_INFO "\thash_for_each_possible test\n");
		//hash_rbtree_for_each_possible(my_hash, nptr, hnode, 30){
			//iterate every node in hashtable my_hash that may contain key==30(?)
			//and store the pointer to the node onto nptr
			//hnode is required to calculate the position of hlist_node in my_node structure.
			
		//	printk(KERN_INFO "value=%d, key=%d is in member=30\n", nptr->value, nptr->hnode.key);
		//	printk("value=%d, key=%d is in member=30\n", nptr->value, nptr->hrb_node.key);
		//}
		printk(KERN_INFO "end search test\n");
		printk(KERN_INFO "begin deletion test");
		//delete
		hash_rbtree_for_each(my_hash, bkt, rptr){
			nptr = rb_entry(rptr, struct my_node, hnode.node);
			rb_erase(&nptr->hnode.node, bucket_for(my_hash, nptr->hnode.key));
			kfree(nptr);
		}
		printk(KERN_INFO "end deletion test");
	}
}



int __init hash_module_init(void)
{
	printk("begin hash module\n");
	hash_example();
	return 0;
}

void __exit hash_module_cleanup(void)
{
	printk("end hash module\n");
}

module_init(hash_module_init);
module_exit(hash_module_cleanup);
MODULE_LICENSE("GPL");

