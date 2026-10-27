#include <unistd.h>

int main() {
	dup(0);
	dup2(0,0);
    close(0);
	getgid();
	sleep(2);
	return 0;
}
