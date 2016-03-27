#include <linux/module.h> 
#include <linux/version.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/kprobes.h> 
#include <net/ip.h> 
//Bringing this back just so that this can compile and I can see things. 

#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

#define NIPQUAD_FMT "%u.%u.%u.%u"

 
int my_handler (struct sk_buff *skb, struct net_device *dev, 
                struct packet_type *pt, struct net_device *orig_dev)
{ 
 
    struct iphdr *my_iph; 
    u32 S_ip,D_ip; 
    my_iph = ip_hdr(skb); 
    S_ip = my_iph->saddr; 
    D_ip = my_iph->daddr; 
    printk("Source IP: "NIPQUAD_FMT"\n",NIPQUAD(S_ip));
    printk("Destination IP: "NIPQUAD_FMT"\n",NIPQUAD(D_ip));  
    jprobe_return(); 
    
    return 0; // just make compiler happy
} 
 
static struct jprobe my_probe; 
 
int myinit(void) 
{ 
    my_probe.kp.addr = (kprobe_opcode_t *)0xffffffff815ecd40; 
    my_probe.entry = (kprobe_opcode_t *)my_handler; 
    register_jprobe(&my_probe); 
    return 0; 
} 
 
void myexit(void) 
{ 
    unregister_jprobe(&my_probe); 
    printk("module removed\n "); 
} 
 
module_init(myinit); 
module_exit(myexit); 
 
/*Kernel module Comments*/
MODULE_AUTHOR("Manoj"); 
MODULE_DESCRIPTION("SIMPLE MODULE"); 
MODULE_LICENSE("GPL"); 
//MODULE_LICENSE("GPL v2");