//
// Created by msimmons on 10/29/26.
//

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

/* kernel module called my_timer calls ktime_get_real_ts64() to obtain current time.
 * Module should store the time value.
 * When my_timer is loaded using insmod, it should create a proc entry named /proc/timer
 * When my_timer is unloaded using rmmod, /proc/timer should be removed.
 * On each read oepration of /proc/timer, utilize proc interface to print both the
 * current time and elapsed time since last call (if valid).
 */

MODULE_LICENSE("GPL");      //this is apparently necessary
MODULE_AUTHOR("The Boys");  //who we are
MODULE_VERSION("1.0");

#define ENTRY_NAME "timer"  //the name the proc file will have
#define PERMS 0444          //the permissions the proc file will have?
#define PARENT NULL

#define BUF_LEN 100

static struct proc_dir_entry *proc_entry; //pointer to proc entry
static struct timespec64 stored_time; //variable to store time

//gets elapsed time by subtracting the stored time from now
struct timespec64 elapsed(struct timespec64 now) {
    struct timespec64 diff;
        diff.tv_sec = now.tv_sec - stored_time.tv_sec;
        diff.tv_nsec = now.tv_nsec - stored_time.tv_nsec;
        if (diff.tv_nsec < 0) {
            diff.tv_sec -= 1;
            diff.tv_nsec += 1000000000L;
        }
    return diff;
}

//function is called when you try to read from /proc/timer
static ssize_t procfile_read(struct file* file, char* usr_buf, size_t count,
    loff_t* pos) {
    printk(KERN_INFO "proc_read\n");
    char buffer[BUF_LEN];
    struct timespec64 now;
    int len;

    if (*pos > 0) {
        return 0;
    }

    ktime_get_real_ts64(&now);
    if (stored_time.tv_sec != 0) {
        struct timespec64 diff = elapsed(now);
        len = snprintf(buffer, sizeof(buffer), "current time: %lld.%09ld elapsed time: %lld.%09ld\n",
            (long long)now.tv_sec, now.tv_nsec, (long long)diff.tv_sec, diff.tv_nsec);
        if (count < len)
            return -EINVAL;
        if (copy_to_user(usr_buf, buffer, len))
            return -EFAULT;
        *pos += len;
    }
    else {
        len = snprintf(buffer, sizeof(buffer), "current time: %lld.%09ld\n", now.tv_sec, now.tv_nsec);
        if (count < len)
            return -EINVAL;
        if (copy_to_user(usr_buf, buffer, len))
            return -EFAULT;
        *pos += len;
    }

    stored_time = now;
    return len;


}


//A structure of function pointers that maps user-space
//filesystem calls on /proc/timer to the kernel module's
//driver callbacks
static const struct proc_ops procfile_fops = {
    .proc_read = procfile_read,
};

//Creates /proc/timer when module is initialized with
//the command: `sudo insmod my_timer.ko`
static int __init my_timer_init(void) {
    proc_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &procfile_fops);
    if (proc_entry == NULL) {
        return -ENOMEM;
    }
        return 0;

}

//Removes /proc/timer when module is removed with
//the command: `sudo rmmod my_timer.ko`
static void __exit my_timer_exit(void) {
    proc_remove(proc_entry);
}

//These two lines tell the kernel which functions to call
//when the module is loaded or removed
module_init(my_timer_init);
module_exit(my_timer_exit);