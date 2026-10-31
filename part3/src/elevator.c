#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/list.h>

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

enum ElevatorState {
	OFFLINE, // elevator isn't running (initial state)
	DEACTIVATING, // elevator is in the process of deactivating
	IDLE, // elevator is stopped on a floor without a pet to service
	LOADING, // elevator is stopped on a floor to load/unload pets
	UP, // elevator is going up
	DOWN, // elevator is going down
};

ElevatorState currentState = ElevatorState.OFFLINE;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int,int,int);
extern int (*STUB_stop_elevator)(void);

static struct proc_dir_entry *proc_entry;

enum PetType { CHIHUAHUA = 3, PUG = 14, PUGHUAHUA = 10, DACHSHUND = 16 };

struct Pet {
	struct list_head node;
	PetType type;
	int dest;
};

struct Pet floors[6]; // floors[0-4] is floors 1-5  and floors[5] is the elevator

int start_elevator(void) {
    if(currentState != OFFLINE) return 1;
	for(int i = 0; i < 6; ++i) {
		INIT_LIST_HEAD(&(floors[i]));
	}
	currentState = IDLE;
    return 0;
}

int issue_request(int start_floor, int destination_floor, int type) {
    Pet* pet;
    pet = kmalloc(sizeof(Pet), _GFP_RECLAIM);
    switch(type) {
        case 0:
		pet->type = PetType.CHIHUAHUA;
		break;
	case 1:
		pet->type = PetType.PUG;
		break;
	case 2:
		pet->type = PetType.PUGHUAHUA;
		break;
	case 3:
		pet->type = PetType.DACHSHUND;
		break;
    }
    pet->dest = destination_floor;
    list_add_tail(&pet->list, &(floors[start_floor-1]));
    return 0;
}

int stop_elevator(void) {
	if(currentState == DEACTIVATING) return 1;
	if(currentState == OFFLINE) return 0;
	for(int i = 0; i < 6; i++) {
		if(floors[i] == nullptr) continue;
		struct Pet *temp;

		list_for_each_entry_safe(item, temp, &(floors[i]), node) {
			list_del(&item->node);
			kfree(item);
		}
	}

    return 0;
}

static int __init elevator_init(void) {
    proc_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &procfile_fops);
    if (proc_entry == NULL) {
        return -ENOMEM;
    }

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
