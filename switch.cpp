#include "switch.h"
#include "switch_internal.h"

float timeVal[NUM_ARRAY];

int main(int argc, char **argv)
{
	int g, rep;

	if(argc != 2)
	{
		printf("RC Control\n usage: switch 2-3 on/off\n");	
	}		
	else
	{
		// Set up gpi pointer for direct register access
		setup_io();

		// Switch GPIO 4 to output mode
		INP_GPIO(4); // must use INP_GPIO before we can use OUT_GPIO
		OUT_GPIO(4);
		
		//Determmine which channel to send the code to
		if(strcmp(argv[1],"2")==0)
		{
			SendCode(2);
		}	
		else if(strcmp(argv[1],"3")==0)
		{
				
			SendCode(3);
		}	
	}

	return 0;
} // main

//
// Set up a memory regions to access GPIO
//
void setup_io()
{
	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0)
	{
		printf("Can't open /dev/mem \n");
		exit(-1);
	}

	/* mmap GPIO */
	// Allocate MAP block
	if ((gpio_mem = (unsigned char *)malloc(BLOCK_SIZE + (PAGE_SIZE - 1))) == NULL)
	{
		printf("Allocation error \n");
		exit(-1);
	}

	// Make sure pointer is on 4K boundary
	if ((unsigned long)gpio_mem % PAGE_SIZE)
	{
		gpio_mem += PAGE_SIZE - ((unsigned long)gpio_mem % PAGE_SIZE);
	}

	// Now map it
	gpio_map = (unsigned char *)mmap(
	   (caddr_t)gpio_mem,
	   BLOCK_SIZE,
	   PROT_READ | PROT_WRITE,
	   MAP_SHARED | MAP_FIXED,
	   mem_fd,
	   GPIO_BASE
	   );

	if ((long)gpio_map < 0)
	{
		printf("mmap error %d\n", (int)gpio_map);
		exit(-1);
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;
} // setup_io

// Function to send the output code to the RF transmitter connected to GPIO 4.
void SendCode(unsigned int ch)
{
	timespec sleeptime;
	timespec remtime;
	unsigned int seed = 0;
	unsigned int base = 1;
	unsigned int numVal = sizeof(timeVal)/sizeof(float);

	// Determine which channel
       
	if(ch == 3)
	{
		memcpy(timeVal, timeVal3, sizeof(float)*NUM_ARRAY);
	}
	else if(ch == 2)
	{
		memcpy(timeVal, timeVal2, sizeof(float)*NUM_ARRAY);
	}
	else
	{
		//return -1;
	}
	

 
	for (int iSend = 0; iSend < loopCount; iSend++)
	{
		base = 1;
		seed = 0;
		sleeptime.tv_sec = 0;
		GPIO_CLR = 1 << 4;
		sleeptime.tv_nsec = timeOff*1000;
		nanosleep(&sleeptime, &remtime);
		
		for (int i = 0; i <numVal ; i++)
		{
			seed ^= base;
			if (seed)
			{

				GPIO_SET = 1 << 4;
			}
			else
			{
				GPIO_CLR = 1 << 4;
			}
			sleeptime.tv_nsec = timeVal[i]*1000;
			nanosleep(&sleeptime, &remtime);
		}
	}
}
