#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Server/lib/unix.h"
#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"
#include "tecnicofs-client-api.c"
#define MAXLINE 512
#define OK 0
#define NO_INPUT 1
#define TOO_LONG 2

/* Global variables */
int sockfd, servlen;
struct sockaddr_un serv_addr;
char socketAddress[MAXLINE];
int fileDescriptor = -1;

/* Lê string de fp e envia para sockfd. Lê string de sockfd e envia para stdout */
void str_cli(FILE *fp, int sockfd)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE + 1];

	while (fgets(sendline, MAXLINE, fp) != NULL)
	{
		/* Envia string para sockfd. Note-se que \0 não é enviado */
		n = strlen(sendline);
		if (write(sockfd, sendline, n) != n)
			printf("str_cli: write error on socket");

		/* Tenta ler string de sockfd. Note-se que tem de terminar a string com \0 */
		n = read(sockfd, recvline, MAXLINE);
		if (n < 0)
			printf("str_cli: read error");
		recvline[n] = 0;

		/* Enviar a string para stdout */
		fputs(recvline, stdout);
	}
	if (ferror(fp))
		printf("str_cli: error reading file");
}

static void displayUsage(const char *appName)
{
	printf("Usage: %s\n", appName);
	exit(EXIT_FAILURE);
}

static void parseArgs(long argc, char *const argv[])
{
	if (argc != 1)
	{
		fprintf(stderr, "Invalid format:\n");
		displayUsage(argv[0]);
	}
}

void menu()
{
	printf("1 - Connect to the server\n");
	printf("2 - Disconnect from the server\n");
	printf("3 - Create file\n");
	printf("4 - Delete file\n");
	printf("5 - Rename file\n");
	printf("6 - Open file\n");
	printf("7 - Close file\n");
	printf("8 - Read file\n");
	printf("9 - Write file\n");
	printf("0 - Quit\n");
	printf("> ");
}

static int getLine(char *prmpt, char *buff, size_t sz)
{
	int ch, extra;

	// Get line with buffer overrun protection.
	if (prmpt != NULL)
	{
		printf("%s", prmpt);
		fflush(stdout);
	}
	if (fgets(buff, sz, stdin) == NULL)
		return NO_INPUT;

	// If it was too long, there'll be no newline. In that case, we flush
	// to end of line so that excess doesn't affect the next call.
	if (buff[strlen(buff) - 1] != '\n')
	{
		extra = 0;
		while (((ch = getchar()) != '\n') && (ch != EOF))
			extra = 1;
		return (extra == 1) ? TOO_LONG : OK;
	}

	// Otherwise remove newline and give string back to caller.
	buff[strlen(buff) - 1] = '\0';
	return OK;
}

void createConnection()
{
	char *socketName;

	// Ask for socket name
	printf("Insert socket name: ");
	scanf("%s", socketName);
	
	int res = tfsMount(socketName); // Replace by user input
	if (res != 0)
		printf("Could not connect to server: %d\n", res);
	else
		printf("Client connected to the server!\n");
}

void destroyConnection()
{
	int res = tfsUnmount();
	if (res != 0)
		printf("The client was already disconnected from the server: %d\n", res);
	else {
		fileDescriptor = -1;
		printf("The client disconnected from the server!\n");
	}	
}

void createFile() {
	char *filename;
	int permissions;
	permission userPermissions, otherPermissions;

	// Ask for filename
	printf("Filename to create: ");
	scanf("%s", filename);

	// Ask for owner permissions
	do {
		printf("User permissions: ");
		scanf("%d", &permissions);
		if(permissions < 0 || permissions > 3)
			printf("Wrong permissions. 0 - No permissions, 1 - Read, 2 - Write, 3 - R/W\n");
	} while(permissions < 0 || permissions > 3);
	userPermissions = permissions;
	
	// Ask for other permissions
	do {
		printf("Other permissions: ");
		scanf("%d", &permissions);
		if(permissions < 0 || permissions > 3)
			printf("Wrong permissions. 0 - No permissions, 1 - Read, 2 - Write, 3 - R/W\n");
	} while(permissions < 0 || permissions > 3);
	otherPermissions = permissions;

	// Call API functions
	int res = tfsCreate(filename, userPermissions, otherPermissions);

	// Process results
	if (res == 0) {
		printf("File created successfully!\n");
	} else if (res == TECNICOFS_ERROR_FILE_ALREADY_EXISTS) {
		printf("File already exists!\n");
	}
}

void deleteFile() {
	
	char *filename;

	// Ask for filename
	printf("Filename to delete: ");
	scanf("%s", filename);

	// Call API function
	int res = tfsDelete(filename);
	if(res < 0)
		printf("Could not write to socket\n");
	else{
		if (res == 0) {
			printf("File sucessfully deleted\n");
		}
		else if(res == TECNICOFS_ERROR_FILE_NOT_FOUND){
			printf("Filename doesn't exist!\n");
		}
	}
}

void renameFile() {

	char *Oldfilename;
	char *Newfilename;

	// Ask for file name
	printf("Filename to change: ");
	scanf("%s", Oldfilename);
	// Ask for new name
	printf("New name for file: ");
	scanf("%s", Newfilename);
	

	// Call API function
	int res = tfsRename(Oldfilename, Newfilename);
	if(res < 0)
		printf("Could not write to socket\n");
	else{
		if (res == 0)
			printf("New name given to file!\n");
		else if(res == TECNICOFS_ERROR_FILE_ALREADY_EXISTS)
			printf("File already exists!\n");
		else if(res == TECNICOFS_ERROR_FILE_NOT_FOUND)
			printf("File does not exists!\n");
	}
}

void openFile() {

	char *filename;
	int permissions;
	permission userPermissions;

	// Ask for file name
	printf("Filename to open: \n");
	scanf("%s", filename);
	// Ask for mode (read, write, both)
	do {
		printf("Open in mode..: \n");
		scanf("%d", &permissions);
		if(permissions < 0 || permissions > 3)
			printf("Wrong permissions. 0 - No permissions, 1 - Read, 2 - Write, 3 - R/W\n");
	} while(permissions < 0 || permissions > 3);
	userPermissions = permissions;

	// Call API function
	int res = 0;//tfsOpen(filename, permissions);
	fileDescriptor = res;
	if(res < 0)
		printf("Could not write to socket\n");
	else{
		if(res == 0)
			printf("File sucessfully opened!\n");
		else if(res == TECNICOFS_ERROR_PERMISSION_DENIED)
			printf("Don't have permissions to open this file!\n");
		else if(res == TECNICOFS_ERROR_MAXED_OPEN_FILES)
			printf("Maximum number of files reached!\n");
		else if(res == TECNICOFS_ERROR_FILE_IS_OPEN)
		printf("File is already open!\n");

	}
}

void closeFile() {

	// Check if we have an opened file
	if(fileDescriptor == -1) {
		printf("There are no opened files.\n");
		return;
	}

	// Call API function
	int res = tfsClose(fileDescriptor);
	if(res < 0)
		printf("Could not write to socket\n");
	else {
		if(res == 0){
			printf("File successfully closed!\n");
		} else  if(res == TECNICOFS_ERROR_FILE_NOT_FOUND) {
			printf("File not found!\n");
		} else if (res == TECNICOFS_ERROR_FILE_NOT_OPEN) {
			printf("File not open!\n");
		}
	}
}

void readFile() {
	
	// Check if we have an opened file
	if(fileDescriptor == -1) {
		printf("There are no opened files.\n");
		return;
	}
	// Create write buffer
	char buffer[MAXLINE];
	// Call API function
	int res = tfsRead(fileDescriptor, buffer, MAXLINE);
	if(res <= 0)
		printf("Could not write to socket\n");
	else{
		if(res == 0) {
		printf("%s", buffer);
		}
	}
}

void writeFile() {
	// Check if we have an opened file
	if(fileDescriptor == -1) {
		printf("There are no opened files.\n");
		return;
	}

	// Create write buffer
	char buffer[MAXLINE];

	// Ask for file contents
	strcpy(buffer, "File contents");

	// Call API function
	int res = tfsWrite(fileDescriptor, buffer, MAXLINE);
	if(res <= 0)
		printf("Could not write to socket\n");
}

int main(int argc, char *argv[])
{
	int option;

	parseArgs(argc, argv); // Validate arguments

	do
	{
		menu();
		scanf("%d", &option);
		if (option == 1)
			createConnection();
		else if (option == 2)
			destroyConnection();
		else if (option == 3)
			createFile();
		else if (option == 4)
			deleteFile();
		else if (option == 5)
			renameFile();
		else if (option == 6)
			openFile();
		else if (option == 7)
			closeFile();
		else if (option == 8)
			readFile();
		else if (option == 9)
			writeFile();
	} while (option != 0);

	/* Disconnect from the server */
	destroyConnection();

	printf("Bye bye!\n");

	exit(0);
}
