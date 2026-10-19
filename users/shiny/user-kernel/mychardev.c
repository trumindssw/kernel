
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/string.h>

#define DEVICE_NAME "my_device"
#define CLASS_NAME  "mychardev"
#define BUFFER_SIZE 256

static dev_t dev_number;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static char message_buffer[BUFFER_SIZE];
static size_t message_length;
static DEFINE_MUTEX(buffer_lock);

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("mychardev: device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    pr_info("mychardev: device closed\n");
    return 0;
}

static ssize_t my_write(struct file *file,
                        const char __user *user_buffer,
                        size_t count, loff_t *offset)
{
    size_t len;
    ssize_t ret;

    if (count == 0)
        return 0;

    len = min(count, (size_t)(BUFFER_SIZE - 1));

    mutex_lock(&buffer_lock);

    if (copy_from_user(message_buffer, user_buffer, len)) {
        ret = -EFAULT;
    } else {
        message_buffer[len] = '\0';
        message_length = len;
        *offset += len;
        pr_info("mychardev: received %zu bytes\n", len);
        ret = len;
    }

    mutex_unlock(&buffer_lock);
    return ret;
}

static ssize_t my_read(struct file *file,
                       char __user *user_buffer,
                       size_t count, loff_t *offset)
{
    size_t available;
    size_t len;
    ssize_t ret;

    mutex_lock(&buffer_lock);

    if (*offset >= message_length) {
        ret = 0;
        goto out;
    }

    available = message_length - (size_t)*offset;
    len = min(count, available);

    if (copy_to_user(user_buffer,
                     message_buffer + *offset, len)) {
        ret = -EFAULT;
        goto out;
    }

    *offset += len;
    ret = len;

out:
    mutex_unlock(&buffer_lock);
    return ret;
}

static const struct file_operations my_fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .read    = my_read,
    .write   = my_write,
    .llseek  = no_llseek,
};

static int __init mychardev_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    cdev_init(&my_cdev, &my_fops);
    my_cdev.owner = THIS_MODULE;

    ret = cdev_add(&my_cdev, dev_number, 1);
    if (ret < 0)
        goto err_unregister;

    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        ret = PTR_ERR(my_class);
        goto err_cdev;
    }

    my_device = device_create(my_class, NULL,
                              dev_number, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        ret = PTR_ERR(my_device);
        goto err_class;
    }

    pr_info("mychardev: registered major=%d minor=%d\n",
            MAJOR(dev_number), MINOR(dev_number));

    return 0;

err_class:
    class_destroy(my_class);
err_cdev:
    cdev_del(&my_cdev);
err_unregister:
    unregister_chrdev_region(dev_number, 1);
    return ret;
}

static void __exit mychardev_exit(void)
{
    device_destroy(my_class, dev_number);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("mychardev: module unloaded\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("User-kernel communication character device");
MODULE_AUTHOR("Linux Kernel Training");
