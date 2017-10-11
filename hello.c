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
#include <linux/wait.h>
#include <linux/spinlock.h>
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
    char *data;
    unsigned long size;
    struct cdev cdev;          //Initialization struct for device of hello
    
};

struct hello_dev_t *hello_dev;


spinlock_t lock;               //declare a spinlock




int hello_open(struct inode *inode,struct file *filp)
{  
    struct hello_dev_t *dev = hello_dev;
    int num = MINOR(inode->i_rdev); //get minor device num

    printk(KERN_NOTICE"====hello_dev open===");
                
    
    if(num >= DEV_DEVS)
          return -ENODEV;
 
    filp->private_data = dev;           //assignment device struct pointer to file private data pointer

    return 0;
}

int hello_release(struct inode *inode,struct file *file)
{
    printk(KERN_NOTICE"====hello_dev release===");
    return 0;
}


static ssize_t hello_read(struct file *filp,char __user *buf,size_t size,loff_t *ppos)
{
    
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;
    
    struct mem_dev *dev;
    dev= filp->private_data;
    printk(KERN_NOTICE"====hello_dev read===");
    if(p >= hello_dev->size)
      return 0;
    if(count > hello_dev-> size-p)
      count = hello_dev-> size-p;
    
    if(copy_to_user(buf,(void *)(hello_dev->data + p),count))
    {
        ret = -EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;
    }
    return ret;
}

static ssize_t hello_write(struct file *filp,const char __user *buf,size_t size,loff_t *ppos)
{
    
    unsigned long p = *ppos;
    unsigned int count = size;
    int ret = 0;    
    struct mem_dev *dev;
    dev = filp->private_data;
    printk(KERN_NOTICE"====hello_dev write===");

    spin_lock(&lock);
/*************************critical zone******************/
    if(p >= hello_dev->size)
      return 0;
    if(count > hello_dev-> size-p)
      count = hello_dev-> size-p;

    if(copy_from_user((void *)(hello_dev->data + p),buf,count))
    {
        ret = -EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;
    }
    return ret;



/********************************************************/
    spin_unlock(&lock);
    
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
     spin_lock_init(&lock);                //Initialization a spinlock

     printk("<0>""hello world,mine cdev is poweron");
     
     if(dev_major)
       res = register_chrdev_region(devno,DEV_DEVS,"hellodev"); //allot device number
     else
     {
       res = alloc_chrdev_region(&devno,0,DEV_DEVS,"hellodev");//动态分配设备编号
       dev_major = MAJOR(devno);           //获取主设备号
     }    
     if(res < 0)
         return res;
    
     hello_dev = kmalloc(DEV_DEVS*sizeof(struct hello_dev_t),GFP_KERNEL);  //allot memory for dev

     if(!hello_dev)
     {
         res = -ENOMEM;               //error num 12
         goto fail_malloc;
     }
  
     memset(hello_dev,0,sizeof(struct hello_dev_t));
     
     cdev_init(&hello_dev->cdev,&hello_dev_fops);        //building connect between in cdev and fops    
 
     hello_dev->cdev.owner = THIS_MODULE;
     hello_dev->cdev.ops = &hello_dev_fops;
    
     cdev_add(&hello_dev->cdev,MKDEV(dev_major,0),DEV_DEVS);

    
     helloclass = class_create(THIS_MODULE,"hello_device");    //creating device file at /dev
     device_create(helloclass,NULL,devno,NULL,"hello_device");
     
     hello_dev->size = DEV_SIZE;
     hello_dev->data = kmalloc(hello_dev->size,GFP_KERNEL);
     memset(hello_dev->data,0,hello_dev->size);
     
     return 0;
  
     fail_malloc:
         unregister_chrdev_region(devno,1);//释放申请的设备号,The number is 1.
     return res;
}




void static hello_exit(void)
{

      printk("<0>""mineworld is poweroff");
      cdev_del(&hello_dev->cdev);                                   //logout cdev
      kfree(hello_dev->data);
      kfree(hello_dev);
      unregister_chrdev_region(MKDEV(dev_major,0),1);
}
//EXPORT_SYMBOL_GPL(device_create);
MODULE_LICENSE("Dual BSD/GPL");
module_init(hello_init);
module_exit(hello_exit);
