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
char *dataBuffer;


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

	//Initilazing Data Buffer
	dataBuffer = (char*) calloc(1,sizeof(char)*BLOCK_SIZE);

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
		fseek(virtualDiskSpace,i*BLOCK_SIZE,SEEK_SET);
		fwrite(buffer,BLOCK_SIZE,1,virtualDiskSpace);
	}

	free(buffer);

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
	writeFat();
	numberOfUsedBlocks++;
	writeDir();
	numberOfUsedBlocks++;
}

// Opens a saved file on the virtual disk and prepares for reading or writing
int vopen(char* filename)
{
	int i=0;
	int fileNumber;
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

	//open file directory saved on virtual disk
	readDir();
	readFat();
	
	//check if filenname is found in file directory
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
	readFat();

	//open file directory saved on virtual disk
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

	//find first empty block for start of file, marsk as first block in FAT table
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
	writeFat();
	writeDir();

	numberOfSavedFiles++;

	//return true inicating file was saved succesfully
	return true;
}

// Close a open file
void vclose(int fd)
{
	fileIsOpen = false;
}

int vread(int fd, int n, char *buffer)
{
	int i = 0;
	int readFromBlock;
	bool continueRead = true;

	//check if a file is open
	if(!fileIsOpen)
	{
		printf("No file is open to read from\n");
		return -1;
	}

	//check if number of bytes in buffer is more than the size of file
	if(fileDirectory[fd].fileSize > n)
	{
		printf("Dataset to read is larger than file\n");
		return -2;
	}

	readFromBlock = fileDirectory[fd].firstBlock;

	//read buffer block by block from virtual disk
	while(continueRead)
	{
		//write data buffer to specified block on virtual disk
		fseek(virtualDiskSpace,readFromBlock*BLOCK_SIZE,SEEK_SET);
		fread(dataBuffer,BLOCK_SIZE,1,virtualDiskSpace);

		//fill dataBuffer with first 1024 char for first block
		strncpy(buffer+i*BLOCK_SIZE,dataBuffer,BLOCK_SIZE);

		//find next block to write
		readFromBlock = fatTable[readFromBlock].nextBlock;

		//check if current block is the last block to write
		if(readFromBlock == LAST_FAT_ENTRY)
		{
			continueRead = false;
		}

		i++;
	}

	return 0;
}

int vwrite(int fd, int n, char *buffer)
{
	int i = 0;
	int writeToBlock;
	bool continueWrite = true;

	//check if a file is open
	if(!fileIsOpen)
	{
		printf("No file is open to write into\n");
		return -1;
	}

	//check if number of bytes in buffer is more than the size of file
	if(fileDirectory[fd].fileSize > n)
	{
		printf("Dataset to write is larger than file can hold\n");
		return -2;
	}
	
	writeToBlock = fileDirectory[fd].firstBlock;


	//write buffer block by block to virtual disk
	while(continueWrite)
	{
		//fill dataBuffer with first 1024 char for first block
		strncpy(dataBuffer,buffer+i*BLOCK_SIZE,BLOCK_SIZE);

		//write data buffer to specified block on virtual disk
		fseek(virtualDiskSpace,writeToBlock*BLOCK_SIZE,SEEK_SET);
		fwrite(dataBuffer,BLOCK_SIZE,1,virtualDiskSpace);

		//find next block to write
		writeToBlock = fatTable[writeToBlock].nextBlock;

		//check if current block is the last block to write
		if(writeToBlock == LAST_FAT_ENTRY)
		{
			continueWrite = false;
		}

		i++;
	}

	return 0;
}

// Displays all filenames in filesystem
void vlist()
{
	int i;

	printf("Files saved in filesystem:\n\n");

	for(i=0;i < MAX_FILES; i++)
	{
		if(strcmp(fileDirectory[i].name,"") != 0)
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
	free(dataBuffer);
}

// Testing filesystem functions
int main()
{
	int i;
	char *input;
	input = (char*) calloc(1,sizeof(char)*2041);

	for (i=0; i < 2040; i++)
	{
		input[i]='A';
	}

	char *output;
	output = (char*) calloc(1,sizeof(char)*2041);

	int filePos = -1;

	printf("starting filesystem\n");
	vinit("disk2.data");
	vformat();

	vsave("file1.data",2040);
	vsave("file2.data",400);
	vsave("file3.data",400);
	filePos = vopen("file1.data");
	printf("File is found at: %i\n",filePos);
	vlist();

	vwrite(filePos,2040,input);
	vread(filePos,2040,output);

	printf("output : %s\n",output);

	printf("\npress ENTER to exit filesystem\n");
	exit();
	getchar();

	return EXIT_SUCCESS;
}
