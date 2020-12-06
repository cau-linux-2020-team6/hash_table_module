#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/ktime.h>

#define MY_HASH_BITS 2

//this file contains the example code for using our reconstructed hashtable,
//which may be faster if the collision occurs too much.
int num_to_test[3] = {1000, 10000, 100000};

struct my_node {
	u32 key;
	int value;
	struct hlist_node hnode;
}

void hash_example(void)
{
	
	DEFINE_HASHTABLE(my_hash, MY_HASH_BITS);
	hash_init(my_hash);
	
	int i;
	//insert
	for(i=0;i<10;i++){
		struct my_node *new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->value = i * 10;
		new->key = i;
		memset(&new->hnode,0,sizeof(struct hlist_node));
		hash_add(my_hash, &new->hnode, new->key);
	}
	//search
		//hash_for_each(...)
	//delete
		//hash_del(...)
}



int __init hash_module_init(void)
{
	printk("begin hash module\n");
	hash_test();
	return 0;
}

void __exit hash_module_cleanup(void)
{
	printk("end hash module\n");
}

module_init(hash_list_module_init);
module_exit(hash_list_module_cleanup);
MODULE_LICENSE("GPL");

