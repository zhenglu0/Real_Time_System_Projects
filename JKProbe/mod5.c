#include <linux/module.h> 
#include <linux/version.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/kprobes.h>
#include <linux/sched.h>  

long my_handler (const char __user *pathname, umode_t mode)
{
    pid_t pid = task_pid_nr(current); 
    printk(KERN_WARNING "The Pathname is %s\n", pathname);
    printk(KERN_WARNING "The PID is %ld\n", (long int)pid);
    jprobe_return(); 
    return 0; // just make compiler happy
} 
 
static struct jprobe my_probe; 
 
int myinit(void) 
{ 
    //my_probe.kp.addr = (kprobe_opcode_t *)0xffffffff811885d0;
    my_probe.kp.addr = (kprobe_opcode_t *)0xffffffff81199070;  
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