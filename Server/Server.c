/* Sistemas Operativos, DEI/IST/ULisboa 2019-20 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "fs.h"
#include "sync.h"
#include "constants.h"
#include "lib/timer.h"
#include "lib/hash.h"
#include "lib/hash.c"
#include "lib/unix.h"
#include "lib/inet.h"
#include "../Client/tecnicofs-api-constants.h"

#define MAXLINE 512

/* Global variables */
char *outputFile = NULL;
int numberBuckets = 0;
#if defined(RWLOCK) || defined(MUTEX)
syncMech commandsLock;
#endif
tecnicofs **hashTable;
int numberCommands = 0;
int headQueue = 0;
char *socketPath = NULL;
int socketId;

static void displayUsage(const char *appName)
{
	printf("Usage: %s <socket name> <output file> <number of buckets>\n", appName);
	exit(EXIT_FAILURE);
}

static void parseArgs(long argc, char *const argv[])
{
	if (argc != 4)
	{
		fprintf(stderr, "Invalid format:\n");
		displayUsage(argv[0]);
	}

	socketPath = argv[1];
	outputFile = argv[2];
	numberBuckets = atoi(argv[3]);

	if (!numberBuckets)
	{
		fprintf(stderr, "Invalid number of buckets\n");
		displayUsage(argv[0]);
	}
}

void initHashTable()
{
	tecnicofs *ht[numberBuckets];
	hashTable = ht;
	for (int i = 0; i < numberBuckets; i++)
		hashTable[i] = new_tecnicofs();
}

FILE *openOutputFile()
{
	FILE *fp;
	fp = fopen(outputFile, "w");
	if (fp == NULL)
	{
		perror("Error opening output file");
		exit(EXIT_FAILURE);
	}
	return fp;
}

void runCommand(char *command)
{
	char token;
	char filename[MAX_INPUT_SIZE];
	sscanf(command, "%c %s", &token, filename);
	//printf("Token: %c, Command: %s", token, command);
	// Calculate hash
	int h = hash(filename, numberBuckets);
	//printf("Hash calculated...\n");

	int iNumber;
	int searchResult;
	//printf("Beginning switch\n");
	switch (token)
	{
		case 'c':
			iNumber = obtainNewInumber(hashTable[h]);
			//printf("iNumber: %d\n", iNumber);
			create(hashTable[h], filename, iNumber);
			//printf("Created %s on BST #%d!\n", filename, h);
			break;
		case 'l':
			searchResult = lookup(hashTable[h], filename); //Erro: a label can only be part of a statement and a declaration is not a statement
			if (!searchResult)
				printf("%s not found\n", filename);
			else
				printf("%s found with inumber %d\n", filename, searchResult);
			break;
		case 'd':
			delete(hashTable[h], filename);
			//printf("Deleted %s on BST #%d!\n", filename, h);
			break;
		default:
		{ /* error */
			//fprintf(stderr, "Error: commands to apply\n");
			printf("Error\n");
			exit(EXIT_FAILURE);
		}
	}
}

void createConnection()
{
	int length;
	struct sockaddr_un socket_address;

	// Get a socket to work with. This socket will be in the UNIX domain, and will be a stream socket.
	if ((socketId = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("server: socket");
		exit(1);
	}

	// Create the address we will be binding to.
	bzero((char *)&socket_address, sizeof(socket_address));
	socket_address.sun_family = AF_UNIX;
	strcpy(socket_address.sun_path, socketPath);

	// Try to bind the address to the socket. We unlink the name first so that the bind won't fail. The third argument indicates the "length" of the structure, not just the length of the socket name.
	unlink(socketPath);
	length = sizeof(socket_address.sun_family) + strlen(socket_address.sun_path);

	if (bind(socketId, (struct sockaddr *)&socket_address, length + 1) < 0)
	{
		perror("server: bind");
		exit(1);
	}

	// Listen on the socket.
	if (listen(socketId, 5) < 0)
	{
		perror("server: listen");
		exit(1);
	}
}

void acceptConnections()
{
	socklen_t fromlen;
	int n, ns, childpid;
	struct sockaddr_un fsaun;
	char command[MAXLINE];

	for (;;)
	{
		// Accept connections. When we accept one, fsaun will be connected to the client. clientSocketAddress will contain the address of the client.
		printf("Accepting new connections...\n");
		if ((ns = accept(socketId, (struct sockaddr *)&fsaun, &fromlen)) < 0)
		{
			perror("server: accept");
			exit(1);
		}
		printf("New connection received!\n");

		// Launch child process to handle client
		if ((childpid = fork()) < 0)
			printf("server: fork error");
		else if (childpid == 0) // Child process has childpid = 0
		{
			printf("New client connected. Child process is waiting for commands...\n");
			close(socketId); // Closes s (unused by the child)
			for (;;)
			{
				// Make sure the client can only open 5 files, by using a vector

				// Read command from the client
				n = read(ns, command, MAXLINE);

				// Execute command in the FS
				command[n] = '\0';
				printf("Received command: %s\n", command);
				//int res = runCommand(command, &buffer);	// Executa o comando do cliente

				// Send response to the client
				// write(ns, res+buffer, 1);
				write(ns, "\0", 1);
			}
			// When the client closes the connection, make sure to close all opened files in the FS
			//exit(0);
		}
		// Processo pai
		// Fecha newsockfd que não utiliza
		close(ns);
	}
}

int main(int argc, char *argv[])
{
	/* Validate arguments */
	parseArgs(argc, argv);

	/* Initialize hash table */
	initHashTable();

	/* Create socket stream and connect */
	createConnection();

	/* Accept new connections from the socket */
	acceptConnections();

	// Wait for the Ctrl+C signal to close the socket connection, wait for child threads to finish, write to the output file and free memory
	// Write in the output file
	/*FILE *outputFp = openOutputFile();
	for (int i = 0; i < numberBuckets; i++)
	{
		print_tecnicofs_tree(outputFp, hashTable[i]);
	}

	// Free memory
	fflush(outputFp);
	fclose(outputFp);
	for (int i = 0; i < numberBuckets; i++)
	{
		free_tecnicofs(hashTable[i]);
	}

	exit(EXIT_SUCCESS);*/
}
