#include "switch.h"
#include "nikhil_switch_3.h"

int main(int argc, char **argv)
{
	int g, rep;

	// Set up gpi pointer for direct register access
	setup_io();

	// Switch GPIO 4 to output mode
	INP_GPIO(4); // must use INP_GPIO before we can use OUT_GPIO
	OUT_GPIO(4);

	SendCode();

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
void SendCode()
{
	timespec sleeptime;
	timespec remtime;
	unsigned int numVal = sizeof(timeVal);


	for (int iSend = 0; iSend < loopCount; iSend++)
	{
		sleeptime.tv_sec = 0;

		GPIO_CLR = 1 << 4;
		//sleeptime.tv_nsec = (timeOff * 1000);
		nanosleep(&sleeptime, &remtime);

		for (int i = 0; i < numVal/*timeSize*/; i++)
		{
			if (i % 2 == 0)
			{

				GPIO_SET = 1 << 4;
			}
			else
			{
				GPIO_CLR = 1 << 4;
			}
			sleeptime.tv_nsec = timeVal[i] * 1000;
			nanosleep(&sleeptime, &remtime);
		}
	}
}
