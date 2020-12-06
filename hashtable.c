#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/ktime.h>

#define MY_HASH_BITS 4

//this file contains the example code for testing original hashtable in linux kernel
int num_to_test[3] = {1000, 10000, 100000};

struct my_node {
	u32 key;
	int value;
	struct hlist_node hnode;
};

void hash_example(void)
{
	int i, j, k;
	int bkt;
	ktime_t tbegin, tend;
	struct my_node* nptr;
	struct hlist_node* hptr;
	DEFINE_HASHTABLE(my_hash, MY_HASH_BITS);
	for(i=0;i<3;i++){
	
		hash_init(my_hash);
			
		//insert
		printk("begin insertion test, size=%d\n", num_to_test[i]);
		tbegin = ktime_get();
		for(j=0;j<num_to_test[i];j++){
			for(k=0;k<2;k++){
				struct my_node *new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
				new->value = j * 10 + k;
				new->key = j;//intended collision
				memset(&new->hnode,0,sizeof(struct hlist_node));
				hash_add(my_hash, &new->hnode, new->key);
			}
		}
		tend = ktime_get();
		printk("insert, %d) Time elapsed:%llu", 2*num_to_test[i], ktime_to_ns(tend - tbegin));
		printk("end insertion test\n");
		printk("begin search test\n");
		printk("\thash_for_each test\n");
		tbegin = ktime_get();
		//search
		hash_for_each(my_hash, bkt, nptr, hnode){
			//iterate every node in hashtable my_hash, and store the pointer to the node onto nptr.
			//bkt means the hashed value.
			//hnode is required to calculate the position of hlist_node in my_node structure.
			
			//printk("value=%d, key=%d is in bucket %d\n", nptr->value, nptr->key, bkt);
		}
		tend = ktime_get();
		printk("hash_for_each, %d) Time elapsed:%llu", 2*num_to_test[i], ktime_to_ns(tend - tbegin));
		printk("\thash_for_each_possible test\n");
		tbegin = ktime_get();
		hash_for_each_possible(my_hash, nptr, hnode, 30){
			//iterate every node in hashtable my_hash that has been inserted on the bucket with the node whose key==30
			//and store the pointer to the node onto nptr
			//hnode is required to calculate the position of hlist_node in my_node structure.
			
			//printk(KERN_INFO "value=%d, key=%d is in member=30\n", nptr->value, nptr->key)
			//printk("value=%d, key=%d is in member=30\n", nptr->value, nptr->key);
		}
		tend = ktime_get();
		printk("hash_for_each_possible, %d) Time elapsed:%llu", 2*num_to_test[i], ktime_to_ns(tend - tbegin));
		printk("end search test\n");
		printk("begin deletion test");
		//delete
		tbegin = ktime_get();
		hash_for_each_safe(my_hash, bkt, hptr, nptr, hnode){
			hash_del(&nptr->hnode);
			kfree(nptr);
		}
		tend = ktime_get();
		printk("delete, %d) Time elapsed:%llu", 2*num_to_test[i], ktime_to_ns(tend - tbegin));
		printk("end deletion test");
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

