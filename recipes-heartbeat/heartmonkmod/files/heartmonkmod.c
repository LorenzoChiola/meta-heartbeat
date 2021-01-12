#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
//#include <asm/uaccess.h>

#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>


/// Error handling macros. An integer variable err in the current scope is utilized.
///   @param func function or expression to check for error.
///   @param label goto label to jump to when an error is detected.
#define errChk(func, label) do{ if((err = (func)) < 0) goto label; }while(0);
#define errChkPtr(func, label) do{ if(IS_ERR(err = (func))) goto label; }while(0);


// Predetermined heart monitor data
#include "data.h"
#define DATALEN (sizeof(ppg) / sizeof(*ppg))	// sizeof(ppg) == 2048 * sizeof(int) 

// Note: The "hm" prefix stands for Heart Monitor
#define MODNAME "heartmonkmod"

static dev_t hm_dev;	// character device structures
struct cdev hm_chardev;
static char buffer[64];

struct class *hm_class = NULL;	// device class (used to create device file automatically)

static wait_queue_head_t hm_waitq;	// wait queue for blocking reads
static int wake_the_read = 0;

static struct hrtimer tim50hz;	// timer to emulate hardware sample rate
static ktime_t period50hz;	// timer period description
static int sample;	// current sample index
static enum hrtimer_restart tim50hz_cb(struct hrtimer * timer);	// 50 Hz timer callback

ssize_t mymod_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int wrcnt=0;
	wait_event_interruptible(hm_waitq, wake_the_read != 0);
	wake_the_read=0;
	//wrcnt = sprintf(buffer, "%d, \t%d.%02d\n", sample/50, (sample%50)*100/50, ppg[sample]); // DEBUG sample index
	wrcnt = sprintf(buffer, "%d\n", ppg[sample]);
	if(wrcnt > count)
		wrcnt = count;
	copy_to_user(buf, buffer, wrcnt);
	
	//printk(KERN_INFO "[%s] read (count=%d, offset=%d)\n", MODNAME, (int)count, (int)*f_pos ); // DEBUG reads
	return wrcnt;
}

// Implemented file operations
struct file_operations hm_dev_fops = {
	.owner = THIS_MODULE,
	.read = mymod_read,
};

static int __init mymod_module_init(void)
{
	int err = 0;
	
	printk(KERN_INFO "[%s] Loading the module...\n", MODNAME);
	
	// Create character device
	errChk(alloc_chrdev_region(&hm_dev, 0, 1, MODNAME),	fail_chrdev);
	//printk(KERN_INFO "[%s]   major: %s\n", MODNAME, format_dev_t(buffer, hm_dev)); // DEBUG major number
	cdev_init(&hm_chardev, &hm_dev_fops);
	hm_chardev.owner = THIS_MODULE;
	errChk(cdev_add(&hm_chardev, hm_dev, 1), fail_chrdevadd);
	
	// Create device class and default device file (/dev/heartmon0)
    errChkPtr(hm_class = class_create(THIS_MODULE, MODNAME"_sys"), fail_class);
    errChkPtr(device_create(hm_class, NULL, hm_dev, NULL, "heartmon0"), fail_devfile);
	
	// Init wait queue for blocking reads
	init_waitqueue_head(&hm_waitq);

	// Init timer (50 Hz tick, emulates interrupts at the sample rate)
	sample = 0;
	period50hz = ktime_set(0, 20*1000*1000); // 0 seconds + 20E6 nanoseconds (20 ms => 50 Hz)
	hrtimer_init(&tim50hz, CLOCK_REALTIME, HRTIMER_MODE_REL);
	tim50hz.function = tim50hz_cb;
	hrtimer_start(&tim50hz, period50hz, HRTIMER_MODE_REL);
	
	return 0;
	
	// Cleanup on error
fail_devfile:
	err = -ENOENT;
	printk(KERN_INFO "[%s]   fail_devfile err: %d\n", MODNAME, err);
	class_destroy(hm_class);
fail_class:
	printk(KERN_INFO "[%s]   fail_class err: %d\n", MODNAME, err);
	cdev_del(&hm_chardev);
fail_chrdevadd:
	printk(KERN_INFO "[%s]   fail_chrdevadd err: %d\n", MODNAME, err);
	unregister_chrdev_region(hm_dev, 1);
fail_chrdev:
	printk(KERN_INFO "[%s]   fail_chrdev err: %d\n", MODNAME, err);
	return err;
}

static void __exit mymod_module_cleanup(void)
{
	printk(KERN_INFO "[%s] Cleaning-up the module.\n", MODNAME);
	
	hrtimer_cancel(&tim50hz);
    device_destroy(hm_class, hm_dev);
    cdev_del(&hm_chardev);
    class_destroy(hm_class);
	unregister_chrdev_region(hm_dev, 1);
}

/// 50 Hz Timer Callback: unblocks the read and resets the timer (akin to an interrupt routine servicing some hardware)
static enum hrtimer_restart tim50hz_cb(struct hrtimer * timer)
{
	// Wake the blocking read
	wake_the_read=1;
	wake_up_interruptible(&hm_waitq);
	
	// After the read is satisfied (if one was blocked)
    sample++;
	if(sample >= DATALEN)
		sample = 0;

    // Reset the timer
	hrtimer_forward_now(timer, period50hz);
    return HRTIMER_RESTART;
}

// Module information registration
module_init(mymod_module_init);
module_exit(mymod_module_cleanup);
MODULE_AUTHOR("Lorenzo Chiola, Massimo Violante");
MODULE_LICENSE("GPL");
