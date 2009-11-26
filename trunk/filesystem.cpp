#include <iostream>
#include <cstdlib>
using namespace std;

const int BLOCK_SIZE = 1024;
const int BLOCK_QTY = 256;
const int EMPTY_FAT_ENTRY = -1;
const int LAST_FAT_ENTRY = -2;
const int MAX_FILES = 35;

// entry in fileDirectory
typedef struct
{
	char name[20];
	int firstBlock;
	int fileSize;

} fileEntry;


// entry in fatTable
typedef struct 
{
	int nextBlock;
	
} fatEntry;


FILE *virtualDiskSpace = NULL;
fatEntry *fatTable;
fileEntry *fileDirectory;



void vinit(char* diskname)
{
	char* buffer;
	int i;

	virtualDiskSpace = fopen(diskname,"rb+");
	if (virtualDiskSpace == NULL)
	{
		printf("new file %s created\n",diskname);
		virtualDiskSpace = fopen(diskname,"wb+");
	}

	buffer = (char*) calloc(1,BLOCK_SIZE);
	for (i=0; i < BLOCK_QTY; i++)
	{
		fwrite(buffer,BLOCK_SIZE,1,virtualDiskSpace);
	}
	free(buffer);

}

void vformat()
{
	int i;

	if (virtualDiskSpace == NULL)
	{
		printf("virtual disk has not been initalized\n");
		printf("Initilazing virtualDisk.data\n");
		vinit("virtualDisk.data");
		printf("Initalization complete\n");
	}
	//Initilazing FAT table for entry into Block 0 in virtudalDiskspace
	fatTable = (fatEntry*) calloc(1,sizeof(fatEntry)*BLOCK_QTY);
	//printf("Size of FAT: %i\n",sizeof(fatTable[0]));
	fatTable[0].nextBlock = LAST_FAT_ENTRY; // Stores FAT table
	fatTable[1].nextBlock = LAST_FAT_ENTRY; // Stores file directory
	for(i=2; i < BLOCK_QTY; i++)
	{
		fatTable[i].nextBlock = EMPTY_FAT_ENTRY;
	}
	
	//Initilazing File Directory table for entry into Block 1 in virtudalDiskspace
	fileDirectory = (fileEntry*) calloc(1,sizeof(fileEntry)*MAX_FILES);

	//Write FAT Table and File Directory into Virtual Disk
	fseek(virtualDiskSpace,0,SEEK_SET);
	fwrite(fatTable,BLOCK_SIZE,1,virtualDiskSpace);
	fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	fwrite(fileDirectory,BLOCK_SIZE,1,virtualDiskSpace);

	fatEntry *buffer;
	buffer = (fatEntry*) calloc(1,sizeof(fatEntry)*BLOCK_QTY);

	fseek(virtualDiskSpace,0,SEEK_SET);
	fread(buffer,BLOCK_SIZE,1,virtualDiskSpace);
	printf("Buffer: %i \n",sizeof(buffer));
	printf("%i\n",buffer[0].nextBlock);


}

int vopen(char* filename)
{

	return 0;
}

void vclose(int fd)
{


}

int vread(int fd, int n, char *buffer)
{

	return 0;
}

int vwrite(int fd, int n, char *buffer)
{

	return 0;
}

void vlist()
{


}

int main()
{

	printf("starting main\n");
	//vinit("disk.data");
	vformat();
	printf("press ENTER to exit main\n");
	getchar();
	return EXIT_SUCCESS;
}
