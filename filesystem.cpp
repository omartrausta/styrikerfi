#include <iostream>
#include <cstdlib>
using namespace std;

const int BLOCK_SIZE = 1024;
const int BLOCK_QTY = 256;
const int EMPTY_ENTRY = -1; // Indicates block is empty and available
const int LAST_FAT_ENTRY = -2; // Indicator for FAT table, last block holding given file
const int MAX_FILES = 35; // the maximum number of files the filesystem can hold
bool fileIsOpen = false;
int numberOfSavedFiles = 0;
int numberOfUsedBlocks = 0;


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


// Sets up virtual disk, if file exsits its opened 
// otherwise it is created, diskname is submitted as input parameter
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

// Formats the virtual disk, if disk is not open it is created using defult filename
// also sets up the FAT table and file directory and saves into first two blocks
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
		fatTable[i].nextBlock = EMPTY_ENTRY;
	}
	
	//Initilazing File Directory table for entry into Block 1 in virtudalDiskspace
	fileDirectory = (fileEntry*) calloc(1,sizeof(fileEntry)*MAX_FILES);
	for(i=0; i < MAX_FILES; i++)
	{
		fileDirectory[i].firstBlock = EMPTY_ENTRY;
	}

	//Write FAT Table and File Directory into Virtual Disk
	fseek(virtualDiskSpace,0,SEEK_SET);
	fwrite(fatTable,BLOCK_SIZE,1,virtualDiskSpace);
	numberOfUsedBlocks++;
	fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	fwrite(fileDirectory,BLOCK_SIZE,1,virtualDiskSpace);
	numberOfUsedBlocks++;

	free(fatTable);
	free(fileDirectory);

	/*
	fatEntry *buffer;
	buffer = (fatEntry*) calloc(1,sizeof(fatEntry)*BLOCK_QTY);

	
	fseek(virtualDiskSpace,0,SEEK_SET);
	fread(buffer,BLOCK_SIZE,1,virtualDiskSpace);
	printf("Buffer: %i \n",sizeof(buffer));
	printf("%i\n",buffer[0].nextBlock);
	*/


}

// Opens a saved file on the virtual disk and prepares for reading or writing
int vopen(char* filename)
{
	int i=2;
	bool fileNotFound = true;

	if(virtualDiskSpace == NULL)
	{
		printf("Virtual disk not initilized, please set up disk\n");
		return -1;
	}
	if(fileIsOpen)
	{
		printf("A file is already open, please close current open file\n");
		return -2;
	}
	

	//check if filenname is found if file dierectory
	while(i < MAX_FILES && fileNotFound)
	{
		if(strcmp(fileDirectory[i].name,filename))
		{
			printf("file found\n");
			fileNotFound = false;
			
		}
		i++;

	}

	
	return -1;
	
}

// Saves a new file to virtual disk
// filename and size of file as input parameters
bool vsave(char *filename, int filesize)
{
	int i = 2;
	int j = 0;
	bool firstBlockNotFound = true;
	bool fileDirEntryNotFound = true;
	fatEntry *fatTableBuffer;
	fileEntry *fileDirectoryBuffer;
	int fileDirEntry = -1;

	//how many blocks are required for file
	int nBlocksNeeded = (float)filesize/(float)BLOCK_SIZE+0.9;

	//check if a disk is initilized
	if(virtualDiskSpace == NULL)
	{
		printf("Virtual disk not initilized, please set up disk\n");
		return false;
	}

	//check if enough space or number of blocks are available on virtual disk
	if(nBlocksNeeded > BLOCK_QTY-numberOfUsedBlocks)
	{
		printf("not enough space available on disk to save file");
		return false;
	}
	//check if file directory is full and more files cannot be added
	if(MAX_FILES == numberOfSavedFiles)
	{
		printf("file directory full, cannot save more files to directory");
		return false;
	}

	//open fat table saved on virtual disk
	fatTableBuffer = (fatEntry*) calloc(1,sizeof(fatEntry)*BLOCK_QTY);
	fseek(virtualDiskSpace,0,SEEK_SET);
	fread(fatTableBuffer,BLOCK_SIZE,1,virtualDiskSpace);

	//open file directory saved on virtual disk
	fileDirectoryBuffer = (fileEntry*) calloc(1,sizeof(fileEntry)*MAX_FILES);
	fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	fread(fileDirectoryBuffer,BLOCK_SIZE,1,virtualDiskSpace);


	//find first empty file directory element
	while (j < MAX_FILES && fileDirEntryNotFound)
	{
		if(fileDirectoryBuffer[j].firstBlock == EMPTY_ENTRY)
		{
			fileDirEntry = j;
			fileDirEntryNotFound = false;
		}
		j++;
	}


	//find first empty block for start of file
	while (i < BLOCK_QTY && firstBlockNotFound)
	{
		if(fatTableBuffer[i].nextBlock == EMPTY_ENTRY)
		{
			fileDirectoryBuffer[fileDirEntry].firstBlock = i;
			fileDirectoryBuffer[fileDirEntry].fileSize = filesize;
			fileDirectoryBuffer[fileDirEntry].name[0] = *filename;
			firstBlockNotFound = false;
		}
		i++;
	}

	//find next available blocks and note block order in FAT table




	return true;

}

// Close a open file, save changes to virtual disk
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

// Testing filesystem functions
int main()
{
	//char testname[20] = "hallo";
	/*int i = 2;
	int max = 10;
	bool logic = true;

	while(i < max && logic)
	{
		printf("logic");
		logic = false;
	}*/


	printf("starting main\n");
	//vinit("disk.data");
	vformat();
	//vopen("file.data");
	vsave("file.data",4000);
	printf("press ENTER to exit main\n");
	getchar();
	return EXIT_SUCCESS;
}
