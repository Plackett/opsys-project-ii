#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("A Pet Elevator used to bring pets up and down between five floors.");
MODULE_VERSION("0.1");

#define ENTRY_NAME "elevator"
#define PERMS 0444
#define PARENT NULL

int start_elevator(void);                                                           // starts the elevator to pick up and drop off passengers
int issue_request(int start_floor, int destination_floor, int type);                // add passengers requests to specific floors
int stop_elevator(void);                                                            // stops the elevator

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int,int,int);
extern int (*STUB_stop_elevator)(void);

static struct proc_dir_entry *proc_entry;

int start_elevator(void) {
    return 0;
}

int issue_request(int start_floor, int destination_floor, int type) {
    return 0;
}

int stop_elevator(void) {
    return 0;
}

static int __init elevator_init(void) {
    proc_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &procfile_fops);
    if (proc_entry == NULL) {
        return -ENOMEM;
    }
    return 0;
    STUB_start_elevator = start_elevator;
	STUB_issue_request = issue_request;
	STUB_stop_elevator = stop_elevator;
    return 0;  // Return 0 to indicate successful loading
}

static void __exit elevator_exit(void) {
    	proc_remove(proc_entry);
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;
}

module_init(elevator_init);  // Specify the initialization function
module_exit(elevator_exit);  // Specify the exit/cleanup function
