#include <iostream>
#include <cstdlib>
using namespace std;


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



int vinit(char* diskname)
{
	char* buffer;
	int i;

	virtualDiskSpace = fopen(diskname,"wb+");
	if (virtualDiskSpace == NULL)
	{
		printf("cannot open file");
		return 1;
	}




	return 0;
}

int vformat()
{

	return 0;
}

int vopen(char* filename)
{

	return 0;
}

int vclose(int fd)
{

	return 0;
}

int vread(int fd, int n, char *buffer)
{

	return 0;
}

int vwrite(int fd, int n, char *buffer)
{

	return 0;
}

int vlist()
{

	return 0;
}

int main()
{

	printf("running");
	vinit("disk.data");
	getchar();
	return EXIT_SUCCESS;
}
