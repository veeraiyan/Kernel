#include <system.h>

addr* page_directory = 0;

/* Maps the block address + 4MB into the paging system
 * with the specified flags set.  Returns the address
 * of the new page table which should be added to the
 * page directory */
addr* page_table_new(addr address, addr flags)
{
	addr i = 0;
	addr* table = (addr*)palloc_aligned(sizeof(addr) * 1024);
	memset((void*)table, 0, sizeof(addr) * 1024);
	for (i = 0; i < 1024; i++)
	{
		table[i] = address | flags; /* Attributes: supervisor level, read/write, present */
		address += 4096; /* Advanced the address to the next page boundry */
	}
	return table;
}

/* Installs the paging system */
void page_install(addr upper)
{
	unsigned char itoa_buffer[256];
	unsigned int cr0;
	int i = 0;

	/* Detect if paging should not be enabled */
	if (upper == 0)
	{
		puts("Paging is not enabled.\n");
		return;
	}

	/* Initalize the page directory area */
	puts("Initializing memory for page directory... ");
	page_directory = (addr*)palloc_aligned(sizeof(addr) * 1024);
	memset((void*)page_directory, 0, sizeof(addr) * 1024);
	puts("done at 0x");
	puts(itoa((addr)page_directory, itoa_buffer, 16));
	puts(".\n");

	/* Set the initial state of the page directory */
	puts("Initializing contents of page directory... ");
	for (i = 0; i < 1024; i++)
	{
		/* Attributes: supervisor level, read/write, not present */
		page_directory[i] = 0 | 2;
	}
	puts("done.\n");

	/* Create the first page table */
	for (i = 0; i < upper / (4 * 1024 * 1024); i += 1)
	{
		puts("Initializing page table for ");
		puts(itoa(i * 4, itoa_buffer, 10));
		puts("MB - ");
		puts(itoa((i + 1) * 4, itoa_buffer, 10));
		puts("MB... ");
		page_directory[i]  = (addr)page_table_new(i * 4 * 1024 * 1024, 3);
		page_directory[i] |= 3;
		puts("done.\n");
	}

	/* Enable paging */
	puts("Enabling paging... ");
	asm volatile("mov %0, %%cr3":: "b"(page_directory));
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
	puts("done.\n");
}