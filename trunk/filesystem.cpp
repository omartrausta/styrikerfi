#include <iostream>
#include <cstdlib>
using namespace std;

const int BLOCK_SIZE = 1024;
const int BLOCK_QTY = 256;

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
	bool isLastBlock;

	
} fatEntry;


FILE* virtualDiskSpace = NULL;
fatEntry* fatTable[255];
fileEntry* fileDirectory;



void vinit(char* diskname)
{
	char* buffer;
	int i;

	virtualDiskSpace = fopen(diskname,"wb+");
	if (virtualDiskSpace == NULL)
	{
		printf("cannot open file\n");
	}
	else
	{
		buffer = (char*) calloc(1,BLOCK_SIZE);
		for (i=0; i < BLOCK_QTY; i++)
		{
			fwrite(buffer,BLOCK_SIZE,1,virtualDiskSpace);
		}
		free(buffer);

	}
}

void vformat()
{


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

	printf("running\n");
	vinit("disk.data");
	printf("press ENTER to exit\n");
	getchar();
	return EXIT_SUCCESS;
}
