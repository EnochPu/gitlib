/********************************************************
*This is a character type device module
*
*made by pavis
*
*
*********************************************************/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#ifndef DEV_MAJOR
#define DEV_MAJOR 66
#endif

#ifndef DEV_DEVS
#define DEV_DEVS 1
#endif

#ifndef DEV_SIZE 
#define DEV_SIZE 4096
#endif

static int dev_major = DEV_MAJOR;

module_param(dev_major,int,S_IRUGO);



struct hello_dev_t{            //define a device struct
     
    struct cdev cdev;          //Initialization struct for device of hello
    
};

struct hello_dev_t *hello_dev;





int hello_open(struct inode *inode,struct file *filp)
{
    printk(KERN_NOTICE"====hello_dev open===");
    return 0;
}

int hello_release(struct inode *inode,struct file *file)
{
    printk(KERN_NOTICE"====hello_dev release===");
    return 0;
}


static ssize_t hello_read(struct file *filp,char __user *buf,size_t count,loff_t *ppos)
{
    printk(KERN_NOTICE"====hello_dev read===");
    return 0;
}

static ssize_t hello_write(struct file *filp,const char __user *buf,size_t count,loff_t *ppos)
{
    printk(KERN_NOTICE"====hello_dev write===");
    return 0;
}

static const struct file_operations hello_dev_fops={         //Setting device operrations 
    .owner = THIS_MODULE,
    .open = hello_open,
    .release = hello_release,
    .read = hello_read,
    .write = hello_write,
    //.ioctl = hello_ioctl,
};

int static hello_init(void)
{
     int res;
     dev_t devno = MKDEV(dev_major,0); 
     struct class *helloclass;

     printk("<0>""hello world,mine cdev is poweron");
     

                         
          printk("<0>""=============init2==================");
     if(dev_major)
       res = register_chrdev_region(devno,DEV_DEVS,"hellodev"); //allot device number
     else
     {
       res = alloc_chrdev_region(&devno,0,DEV_DEVS,"hellodev");
       dev_major = MAJOR(devno);
     }    
     if(res < 0)
         return res;
    

     printk("<0>""=============init4==================");
     hello_dev = kmalloc(DEV_DEVS*sizeof(struct hello_dev_t),GFP_KERNEL);  //allot memory for dev
     printk("<0>""=============init5==================");
     if(!hello_dev)
     {
         res = -ENOMEM;
         goto fail_malloc;
     }
     printk("<0>""=============init6==================");
     memset(hello_dev,0,sizeof(struct hello_dev_t));
     
     cdev_init(&hello_dev->cdev,&hello_dev_fops);        //building connect between in cdev and fops    
     printk("<0>""=============init1==================");
     hello_dev->cdev.owner = THIS_MODULE;
     hello_dev->cdev.ops = &hello_dev_fops;
     printk("<0>""=============init3==================");
     cdev_add(&hello_dev->cdev,MKDEV(dev_major,0),DEV_DEVS);

    
     helloclass = class_create(THIS_MODULE,"hello_device");    //creating device file at /dev
     device_create(helloclass,NULL,devno,NULL,"hello_device");
     
     return 0;
  
     fail_malloc:
         unregister_chrdev_region(devno,1);
     return res;
}




void static hello_exit(void)
{

      printk("<0>""mineworld is poweroff");
      cdev_del(&hello_dev->cdev);                                   //logout cdev
      kfree(hello_dev);
      unregister_chrdev_region(MKDEV(dev_major,0),1);
}
//EXPORT_SYMBOL_GPL(device_create);
MODULE_LICENSE("Dual BSD/GPL");
module_init(hello_init);
module_exit(hello_exit);
