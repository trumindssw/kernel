#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/proc_fs.h> 
#include <linux/uaccess.h> 
#include <linux/string.h> 
 
#define PROC_NAME "my_user_kernel" 
#define BUF_SIZE  256 
 
static struct proc_dir_entry *proc_entry; 
static char message[BUF_SIZE] = 
    "Hello from kernel space!"; 
 
static ssize_t proc_read(struct file *file, 
                         char __user *user_buf, 
                         size_t count, 
                         loff_t *ppos) 
{ 
    return simple_read_from_buffer( 
        user_buf, count, ppos, 
        message, strlen(message)); 
} 
 
static ssize_t proc_write(struct file *file, 
                          const char __user *user_buf, 
                          size_t count, 
                          loff_t *ppos) 
{ 
    size_t len = min(count, (size_t)BUF_SIZE - 1); 
 
    if (copy_from_user(message, user_buf, len)) 
        return -EFAULT; 
 
    message[len] = '\0'; 
 
    if (len > 0 && message[len - 1] == '\n') 
        message[--len] = '\0'; 
 
    pr_info("proc_ipc: received from user: %s\n", 
            message); 
 
    return count; 
} 
 
static const struct proc_ops proc_fops = { 
    .proc_read  = proc_read, 
    .proc_write = proc_write, 
}; 
 
static int __init proc_ipc_init(void) 
{ 
    proc_entry = proc_create( 
        PROC_NAME, 0666, NULL, &proc_fops); 
 
    if (!proc_entry) 
        return -ENOMEM; 
 
    pr_info("proc_ipc: /proc/%s created\n", 
            PROC_NAME); 
 
    return 0; 
} 
 
static void __exit proc_ipc_exit(void) 
{ 
    proc_remove(proc_entry); 
    pr_info("proc_ipc: module unloaded\n"); 
} 
 
module_init(proc_ipc_init); 
module_exit(proc_ipc_exit); 
 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Revathi"); 
MODULE_DESCRIPTION("User-kernel communication using procfs"); 
