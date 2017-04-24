#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
 
#define BRAM0_BASE_ADDRESS     0x40000000
#define BRAM1_BASE_ADDRESS     0x42000000
#define GPIO_DATA_OFFSET     0
#define GPIO_DIRECTION_OFFSET     4
 
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
 
int main()
{
 	int memfd;
	int i;
	void *bram0_base, *mmaped0_dev_base;
	void *bram1_base, *mmaped1_dev_base;
	off_t bram0_dev_base = BRAM0_BASE_ADDRESS;
	off_t bram1_dev_base = BRAM1_BASE_ADDRESS;

	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		exit(0);
	}
	printf("/dev/mem opened.\n");
										    
	bram0_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, bram0_dev_base & ~MAP_MASK);
	if (bram0_base == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}
	printf("Memory mapped at address %p.\n", bram0_base);
														      
	bram1_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, bram1_dev_base & ~MAP_MASK);
	if (bram1_base == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		exit(0);
	}
	printf("Memory mapped at address %p.\n", bram0_base);
	mmaped0_dev_base = bram0_base + (bram0_dev_base & MAP_MASK);
	mmaped1_dev_base = bram1_base + (bram1_dev_base & MAP_MASK);
	*((volatile unsigned long *) (mmaped0_dev_base + GPIO_DIRECTION_OFFSET)) = 0;
	*((volatile unsigned long *) (mmaped1_dev_base + GPIO_DIRECTION_OFFSET)) = 1;
																							  
	for(i=0; i<10; i++) {
		*((volatile unsigned long *) (mmaped0_dev_base + (GPIO_DATA_OFFSET+(4*i)))) = 0x11223344+i;
	}

	sleep(10);
	for(i=0; i<10; i++) {
		printf("%lx -> %lx\n",(volatile unsigned long *) (mmaped1_dev_base + (GPIO_DATA_OFFSET+(4*i))),*((volatile unsigned long *) (mmaped1_dev_base + (GPIO_DATA_OFFSET+(4*i)))));
	}
																								     

	if (munmap(bram0_base, MAP_SIZE) == -1) {
		printf("Can't unmap memory from user space.\n");
		exit(0);
	}
	if (munmap(bram1_base, MAP_SIZE) == -1) {
		printf("Can't unmap memory from user space.\n");
		exit(0);
	}
	close(memfd);
	return 0;
}
