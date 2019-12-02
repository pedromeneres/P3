#include <sys/un.h>

#define MAXLINE 512

/* Global variables */
int socketId;

int tfsMount(char *socketPath)
{
	int length, connection;
	struct sockaddr_un socket_address;

	// Get a socket to work with. This socket will be in the UNIX domain, and will be a stream socket.
	socketId = socket(AF_UNIX, SOCK_STREAM, 0);
	if (socketId < 0)
		return socketId; // Return error
	printf("Socket stream created\n");

	// Create the address we will be binding to.
	bzero((char *)&socket_address, sizeof(socket_address));
	socket_address.sun_family = AF_UNIX;
	strcpy(socket_address.sun_path, "socket");

	// Try to bind the address to the socket. We unlink the name first so that the bind won't fail. The third argument indicates the "length" of the structure, not just the length of the socket name.
	length = strlen(socket_address.sun_path) + sizeof(socket_address.sun_family);
	connection = connect(socketId, (struct sockaddr *)&socket_address, length + 1);
	if (connection < 0)
		return connection; // Return connection in case of error
	return 0;
}

int tfsUnmount()
{
	close(socketId);
	return 0;
}

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions) {
	// Create command string
	char command[MAXLINE];
	strcpy(command, "c ");
	strcat(command, filename);
	strcat(command, " ");
	strcat(command, "3"); // Owner permissions
	strcat(command, "2"); // Other permissions

	// Send command to socket/server
	return write(socketId, command, MAXLINE);

	// Get response from the socket/server and return it
	
}

int tfsDelete(char *filename) {
	// Create command string
	char command[MAXLINE];
	strcpy(command, "d ");
	strcat(command, filename);

	// Send command to socket/server
	return write(socketId, command, MAXLINE);

	// Get response from the socket/server and return it
}

int tfsRename(char *filenameOld, char *filenameNew) {
	// Create command string
	char command[MAXLINE];
	strcpy(command, "r ");
	strcat(command, filenameOld);
	strcat(command, " ");
	strcat(command, filenameNew);

	// Send command to socket/server
	return write(socketId, command, MAXLINE);

	// Get response from the socket/server and return it
}

int tfsOpen(char *filename, permission mode) {
	// Create command string
	char command[MAXLINE];
	strcpy(command, "o ");
	strcat(command, filename);
	strcat(command, " ");
	strcat(command, "3");

	// Send command to socket/server
	return write(socketId, command, MAXLINE);

	// Get response from the socket/server and return it
	int n = read(socketId, command, MAXLINE); // Lê a responsta do servidor
	int res = atoi(command);
	if(res == TECNICOFS_ERROR_FILE_NOT_FOUND) {
		printf("File not found.\n");
	} else if (res == TECNICOFS_ERROR_PERMISSION_DENIED) {
		printf("Permission denied.\n");
	} else if (res == TECNICOFS_ERROR_MAXED_OPEN_FILES) {
		printf("Cannot open any more files.\n");
	}
}

int tfsClose(int fd) {
	// Create command string
	char command[MAXLINE];
	strcpy(command, "x ");
	strcat(command, "5"); // Convert fd to string/char

	// Send command to socket/server
	return write(socketId, command, MAXLINE);

	// Get response from the socket/server and return it
}

int tfsRead(int fd, char *buffer, int len) {
	// Create command string
	char command[MAXLINE];
	strcpy(command, "l ");
	strcat(command, "5 "); // Convert fd to string
	strcat(command, "512"); // Convert len to string

	// Send command to socket/server
	return write(socketId, command, MAXLINE);

	// Get response from the socket/server and copy it to the buffer

	// Return the number of characters received
}

int tfsWrite(int fd, char *buffer, int len) {
	// Create command string
	char command[MAXLINE];
	strcpy(command, "w ");
	strcat(command, "5 "); // Convert fd to string
	strcat(command, buffer); // Convert len to string

	// Send command to socket/server
	return write(socketId, command, MAXLINE);

	// Get response from the socket/server and return it
}