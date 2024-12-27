#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time.h>

MODULE_DESCRIPTION("Kernel Module");
MODULE_AUTHOR("FarenbruhBoris");

#define PROCFS_NAME "lab"
#define LAUNCH_YEAR 1977
#define LAUNCH_MONTH 8
#define LAUNCH_DAY 20
#define LAUNCH_HOUR 21
#define LAUNCH_MINUTE 39

static struct proc_dir_entry *proc_file = NULL;
static struct timespec64 start_time;

static long days_since_VoyagerStart(void) {
    struct timespec64 current_time;
    long long diff_sec;
    long long days;

    ktime_get_real_ts64(&current_time); 

    diff_sec = current_time.tv_sec - start_time.tv_sec; 
    days = diff_sec / (60 * 60 * 24); 

    return days;
}


static ssize_t proc_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    char output[32];
    size_t output_len;
    long days;

    if (*offset > 0)
        return 0;

    days = days_since_VoyagerStart(); 

    output_len = snprintf(output, sizeof(output), "%ld\n", days); 
    if (copy_to_user(buffer, output, output_len))
        return -EFAULT;

    *offset += output_len;
    return output_len;
}


static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
};

static int __init tsu_init(void)
{
    struct tm start_tm;
    time64_t start_time_sec;

    start_tm.tm_year = LAUNCH_YEAR - 1900;
    start_tm.tm_mon = LAUNCH_MONTH - 1;
    start_tm.tm_mday = LAUNCH_DAY;
    start_tm.tm_hour = LAUNCH_HOUR;
    start_tm.tm_min = LAUNCH_MINUTE;
    start_tm.tm_sec = 0;
    
    
    start_time_sec = mktime64(start_tm.tm_year + 1900, start_tm.tm_mon + 1, start_tm.tm_mday,
                             start_tm.tm_hour, start_tm.tm_min, start_tm.tm_sec);


    start_time.tv_sec = start_time_sec;
    start_time.tv_nsec = 0;



    proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_ops);
    if (!proc_file) {
        pr_err("Failed to create /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit tsu_exit(void)
{
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(tsu_init);
module_exit(tsu_exit);

MODULE_LICENSE("GPL");