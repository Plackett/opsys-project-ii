#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("A Pet Elevator used to bring pets up and down between five floors.");
MODULE_VERSION("0.1");

#define ENTRY_NAME "elevator"
#define PERMS 0444
#define PARENT NULL
#define BUF_SIZE 10000

int start_elevator(void);                                                           // starts the elevator to pick up and drop off passengers
int issue_request(int start_floor, int destination_floor, int type);                // add passengers requests to specific floors
int stop_elevator(void);                                                            // stops the elevator

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int,int,int);
extern int (*STUB_stop_elevator)(void);

static struct proc_dir_entry *proc_entry;

// this enum stores pet types by their weight on the elevator
enum PetType { CHIHUAHUA = 3, PUG = 14, PUGHUAHUA = 10, DACHSHUND = 16 };

struct Pet {
    struct list_head node; // this is from list.h and just has two list_heads of its own, one for prev and one for next
    enum PetType type;
    int dest;
};

// structure that just keeps a linked list and additional info
struct pet_list {
    struct list_head head;
    struct mutex mutex;
    int size;    // size of the list
    int C,D,P,H; // count of each type of pet in the list
};
struct pet_list floor[6]; // floor[1-5] is floors 1-5  and floor[0] is the elevator

enum ElevatorState {
    OFFLINE, // elevator isn't running (initial state)
    IDLE, // elevator is stopped on a floor without a pet to service
    LOADING, // elevator is stopped on a floor to load/unload pets
    UP, // elevator is going up
    DOWN, // elevator is going down
};

struct Elevator {
    enum ElevatorState state;
    struct list_head head;
    struct task_struct* kthread;
    struct mutex mutex;
    int size;
    int weight;
    int floor;
    bool deactivating;
};
struct Elevator elevator;

char* state_as_string(void) {
    switch(elevator.state) {
        case OFFLINE:
            return "OFFLINE";
        case IDLE:
            return "IDLE";
        case LOADING:
            return "LOADING";
        case UP:
            return "UP";
        case DOWN:
            return "DOWN";
    };
}

int thread_run_elevator(void* param) {
    struct elevator* elevator = param;
    while(!kthread_should_stop()) {
    }
}

// start elevator sets the state to IDLE if the elevator is offline and returns an error (1) if the elevator has already been started
// it also resets the deactivating flag to false
int start_elevator(void) {
    if(elevator.state != OFFLINE) return 1;

    INIT_LIST_HEAD(&elevator.head);
    elevator.state = IDLE;
    elevator.kthread = kthread_run(thread_run_elevator, &elevator, "elevator");

    return 0;
}

// issue request adds a pet to the list for that floor it is on and stores their destination and pet type
// kmalloc could potentially fail here
int issue_request(int start_floor, int destination_floor, int type) {
    if(start_floor < 1 || start_floor > 5 || destination_floor < 1 || destination_floor > 5 || type < 0 || type > 3) return 1;
    struct Pet* pet;
    pet = kmalloc(sizeof(struct Pet), GFP_KERNEL);
    if(pet == NULL) return -ENOMEM; // failed memory initailizations should return ENOMEM
    switch(type) {
        case 0:
            pet->type = CHIHUAHUA;
            floor[start_floor].C++;
            break;
        case 1:
            pet->type = PUG;
            floor[start_floor].P++;
            break;
        case 2:
            pet->type = PUGHUAHUA;
            floor[start_floor].H++;
            break;
        case 3:
            pet->type = DACHSHUND;
            floor[start_floor].D++;
            break;
    }
    pet->dest = destination_floor;
    // void list_add_tail(struct list_head *new, struct list_head *head);

    // Wait until lock aquired
    while(!mutex_lock_interruptible(&floor[start_floor].mutex)) {;}

    list_add_tail(&pet->node, &floor[start_floor].head);
    floor[start_floor].size++;

    mutex_unlock(&floor[start_floor].mutex);
    return 0;
}

// stop elevator just triggers the deactivating flag and returns an error(1) if its already deactivating when called
// if the elevator is already offline it won't do anything
int stop_elevator(void) {
    if(elevator.deactivating == true) return 1;
    if(elevator.state == OFFLINE) return 0;
    elevator.deactivating = true;
    return 0;
}

// TODO this function :)
// called when reading /proc/elevator
static ssize_t procfile_read(struct file* file, 
        char* ubuf,
        size_t count, 
        loff_t* ppos) {
    char buffer[BUF_SIZE];
    int len = 0;
    /*
    struct pet_count petinfo[6];
    for (int i = 1; i <= 5; i++) {
        // initialize values
        pet_list* f;
    }
    snprintf(buffer, sizeof(buffer), "Elevator state: <state> Current floor: <floor> Current load: <weight> lbs Elevator status: ")
        //C<num_c> D<num_d> P<num_p> H<num_h> [<f5?>] Floor 5: <peton5> H3 [ ] Floor 4: 0 [*] Floor 3: 2 P5 C1 [ ] Floor 2: 1 D4 [ ] Floor 1: 0 Number of pets: 4 Number of pets waiting: 4 <waiting> Number of pets serviced: <serviced>")
    */

    return 0;
}

static const struct proc_ops procfile_fops = {
    .proc_read = procfile_read,
};

// kernel module entry function
// initializes the linked lists for every floor
static int __init elevator_init(void) {
    // Initialize elevator
    elevator.size = 0;
    elevator.weight = 0;
    elevator.floor = 1;
    elevator.deactivating = false;
    elevator.state = OFFLINE;
    mutex_init(&elevator.mutex);

    // Initialize floors
    for(int i = 1; i < 6; ++i) {
        // void INIT_LIST_HEAD(struct list_head *list);
        INIT_LIST_HEAD(&floor[i].head);
        floor[i].C = 0;
        floor[i].D = 0;
        floor[i].P = 0;
        floor[i].H = 0;
        mutex_init(&floor[i].mutex);
    }

    proc_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &procfile_fops);
    if (proc_entry == NULL) return -ENOMEM;

    return 0;  // Return 0 to indicate successful loading
}

// kernel module exit function
// frees all of the floor lists and exits safely
static void __exit elevator_exit(void) {
    struct Pet *item, *temp;

    proc_remove(proc_entry);

    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    for(int i = 0; i < 6; i++) {

        // list_for_each_entry_safe(struct list_head* pos, struct list_head* head, member);
        list_for_each_entry_safe(item, temp, &floor[i].head, node) {
            // void list_del(struct list_head *entry);
            list_del(&item->node);
            kfree(item);
        }
    }
}

module_init(elevator_init);  // Specify the initialization function
module_exit(elevator_exit);  // Specify the exit/cleanup function
