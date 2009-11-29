#include <iostream>
#include <cstdlib>
#include <string.h>
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


// Helper functions for reading and writing FAT and File Directory to and from filesystem
// For use in other functions not main program

void readFat()
{
	//fatTable = (fatEntry*) calloc(1,sizeof(fatEntry)*BLOCK_QTY);
	fseek(virtualDiskSpace,0,SEEK_SET);
	fread(fatTable,BLOCK_SIZE,1,virtualDiskSpace);
}

void writeFat()
{
	fseek(virtualDiskSpace,0,SEEK_SET);
	fwrite(fatTable,BLOCK_SIZE,1,virtualDiskSpace);
	//free(fatTable);
}

void readDir()
{
	//fileDirectory = (fileEntry*) calloc(1,sizeof(fileEntry)*MAX_FILES);
	fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	fread(fileDirectory,BLOCK_SIZE,1,virtualDiskSpace);
}

void writeDir()
{
	fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	fwrite(fileDirectory,BLOCK_SIZE,1,virtualDiskSpace);
	//free(fileDirectory);
}

//Functions for use in main program


// Sets up virtual disk, if file exsits its opened 
// otherwise it is created, diskname is submitted as input parameter
// Initilaze memory location for fatTable and fileDirectory
void vinit(char* diskname)
{
	int i;

	virtualDiskSpace = fopen(diskname,"rb+");
	if (virtualDiskSpace == NULL)
	{
		printf("new file %s created\n",diskname);
		virtualDiskSpace = fopen(diskname,"wb+");
	}

	//Initilazing FAT table for entry into Block 0 in virtudalDiskspace
	fatTable = (fatEntry*) calloc(1,sizeof(fatEntry)*BLOCK_QTY);

	//Initilazing File Directory table for entry into Block 1 in virtudalDiskspace
	fileDirectory = (fileEntry*) calloc(1,sizeof(fileEntry)*MAX_FILES);

}

// Formats the virtual disk, if disk is not open it is created using defult filename
// also sets up the FAT table and file directory and saves them into first two blocks
void vformat()
{
	int i;
	char* buffer;

	if (virtualDiskSpace == NULL)
	{
		printf("virtual disk has not been initalized\n");
		printf("Initilazing virtualDisk.data\n");
		vinit("virtualDisk.data");
		printf("Initalization complete\n");
	}

	//Empty all blocks in filesystem
	buffer = (char*) calloc(1,BLOCK_SIZE);
	for (i=0; i < BLOCK_QTY; i++)
	{
		fseek(virtualDiskSpace,i,SEEK_SET);
		fwrite(buffer,BLOCK_SIZE,1,virtualDiskSpace);
	}
	free(buffer);

	//printf("Size of FAT: %i\n",sizeof(fatTable[0]));
	fatTable[0].nextBlock = LAST_FAT_ENTRY; // Stores FAT table
	fatTable[1].nextBlock = LAST_FAT_ENTRY; // Stores file directory
	for(i=2; i < BLOCK_QTY; i++)
	{
		fatTable[i].nextBlock = EMPTY_ENTRY;
	}
	

	for(i=0; i < MAX_FILES; i++)
	{
		fileDirectory[i].firstBlock = EMPTY_ENTRY;
	}

	//Write FAT Table and File Directory into Virtual Disk
	//fseek(virtualDiskSpace,0,SEEK_SET);
	//fwrite(fatTable,BLOCK_SIZE,1,virtualDiskSpace);
	writeFat();
	numberOfUsedBlocks++;
	//fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	//fwrite(fileDirectory,BLOCK_SIZE,1,virtualDiskSpace);
	writeDir();
	numberOfUsedBlocks++;

	//free(fatTable);
	//free(fileDirectory);

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
	int i=0;
	int fileNumber;
	bool fileNotFound = true;
	//fileEntry *fileDirectoryBuffer;

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

	//open file directory saved on virtual disk
	//fileDirectoryBuffer = (fileEntry*) calloc(1,sizeof(fileEntry)*MAX_FILES);
	//fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	//fread(fileDirectoryBuffer,BLOCK_SIZE,1,virtualDiskSpace);
	readDir();
	
	//check if filenname is found in file dierectory
	while(i < MAX_FILES && fileNotFound)
	{
		if(strcmp(fileDirectory[i].name,filename) == 0)
		{
			//printf("file %s found\n",fileDirectoryBuffer[i].name);
			fileNumber = i;
			fileNotFound = false;			
		}
		i++;
	}
	
	if(fileNotFound)
	{
		printf("File could not be found\n");
		return -3;
	}

	//DEV NOTE: VISUAL STUDIO MAY THROW BREAKBPOINT, CONTINUE WILL COMPLETE PROGRAM RUN SUCCESSFULLY
	//free(fileDirectoryBuffer);

	//return file number reference for location of file within filesystem, mark file as open
	fileIsOpen = true;
	return fileNumber;
	
}

// Saves a new file to virtual disk
// filename and size of file as input parameters
bool vsave(char *filename, int filesize)
{
	int i = 2;
	int j = 0;
	bool firstBlockNotFound = true;
	bool fileDirEntryNotFound = true;
	//fatEntry *fatTableBuffer;
	//fileEntry *fileDirectoryBuffer;
	int fileDirEntry = -1;
	int lastBlock = 0;

	//how many blocks are required for file
	int nBlocksNeeded = (float)filesize/(float)BLOCK_SIZE+0.9;

	//check if a disk is initilized
	if(virtualDiskSpace == NULL)
	{
		printf("Virtual disk not initilized, please set up disk\n");

		//return false indicating file was unable to be saved succesfully
		return false;
	}

	//check if enough space or number of blocks are available on virtual disk
	if(nBlocksNeeded > BLOCK_QTY-numberOfUsedBlocks)
	{
		printf("not enough space available on disk to save file\n");
		return false;
	}

	//check if file directory is full and more files cannot be added
	if(MAX_FILES == numberOfSavedFiles)
	{
		printf("file directory full, cannot save more files to directory\n");
		return false;
	}

	//open fat table saved on virtual disk
	//fatTableBuffer = (fatEntry*) calloc(1,sizeof(fatEntry)*BLOCK_QTY);
	//fseek(virtualDiskSpace,0,SEEK_SET);
	//fread(fatTableBuffer,BLOCK_SIZE,1,virtualDiskSpace);
	readFat();

	//open file directory saved on virtual disk
	//fileDirectoryBuffer = (fileEntry*) calloc(1,sizeof(fileEntry)*MAX_FILES);
	//fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	//fread(fileDirectoryBuffer,BLOCK_SIZE,1,virtualDiskSpace);
	readDir();

	//check if file already exists with same filename
	for(j=0;j < MAX_FILES; j++)
	{
		if(strcmp(fileDirectory[j].name,filename) == 0)
		{
			printf("filename already exists\n");
			return false;
		}
	}


	//find first empty file directory element
	j = 0;
	while (j < MAX_FILES && fileDirEntryNotFound)
	{
		if(fileDirectory[j].firstBlock == EMPTY_ENTRY)
		{
			fileDirEntry = j;
			fileDirEntryNotFound = false;
		}
		j++;
	}

	//find first empty block for start of file
	while (i < BLOCK_QTY && firstBlockNotFound)
	{
		if(fatTable[i].nextBlock == EMPTY_ENTRY)
		{
			fileDirectory[fileDirEntry].firstBlock = i;
			fileDirectory[fileDirEntry].fileSize = filesize;
			memcpy(fileDirectory[fileDirEntry].name,filename,sizeof(fileDirectory[fileDirEntry].name));
			firstBlockNotFound = false;
			nBlocksNeeded--;
			lastBlock = i;
		}
		i++;
	}

	//find next available blocks and note block order in FAT table, note last block as LAST_FAT_ENTRY
	while (i < BLOCK_QTY && nBlocksNeeded >= 0)
	{
		if(fatTable[i].nextBlock == EMPTY_ENTRY && nBlocksNeeded > 0)
		{
			fatTable[lastBlock].nextBlock = i;

			lastBlock = i;
		}
		if(nBlocksNeeded == 0)
		{
			fatTable[lastBlock].nextBlock = LAST_FAT_ENTRY;

		}

		numberOfUsedBlocks++;
		nBlocksNeeded--;
		i++;
	}

	//save FAT table and file directory to file
	//fseek(virtualDiskSpace,0,SEEK_SET);
	//fwrite(fatTableBuffer,BLOCK_SIZE,1,virtualDiskSpace);
	//fseek(virtualDiskSpace,1*BLOCK_SIZE,SEEK_SET);
	//fwrite(fileDirectoryBuffer,BLOCK_SIZE,1,virtualDiskSpace);
	writeFat();
	writeDir();

	//free(fatTableBuffer);
	//free(fileDirectoryBuffer);

	numberOfSavedFiles++;

	//return true inicating file was saved succesfully
	return true;

}

// Close a open file, save changes to virtual disk
void vclose(int fd)
{
	fileIsOpen = false;


}

int vread(int fd, int n, char *buffer)
{

	return 0;
}

int vwrite(int fd, int n, char *buffer)
{

	return 0;
}

// Displays all filenames in filesystem
void vlist()
{
	int i;

	printf("Files saved in filesystem:\n\n");

	for(i=0;i < MAX_FILES; i++)
	{
		if(fileDirectory[i].firstBlock != EMPTY_ENTRY)
		{
			printf("%s\n",fileDirectory[i].name);
		}
	} 
}

//Cleans up and frees allocated memory
void exit()
{
	free(fatTable);
	free(fileDirectory);
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

	/*
	int *array1;
	array1 = (int*) calloc(1,sizeof(int)*8);
	array1[0]=1;
	array1[1]=2;
	array1[2]=3;
	
	printf("array1 stak 2: %i\n",array1[2]);

	int array2[8];

	memcpy(array2,array1,sizeof(array2));

	//array2 = array1;

	printf("array2 stak 2: %i\n",array2[2]);
	*/


	int filePos = -1;






	printf("starting filesystem\n");
	vinit("disk2.data");
	vformat();

	vsave("file1.data",4000);
	vsave("file2.data",400);
	vsave("file3.data",400);
	filePos = vopen("file3.data");
	printf("File is found at: %i\n",filePos);
	vlist();
	printf("\npress ENTER to exit filesystem\n");
	exit();
	getchar();
	return EXIT_SUCCESS;
}
