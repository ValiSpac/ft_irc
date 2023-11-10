#include "Server.hpp"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <cerrno>


const int BACKLOG = 5;
const int BUFFER_SIZE = 1024;

irc::Server::Server(int port, std::string password) :
	_port(port),
	_password(password)
{
	this->_backlog = BACKLOG;
	this->_bufferSize = BUFFER_SIZE;
	this->_socket = 0;
}

irc::Server::~Server()
{
}

irc::Server::Server(const Server& other)
{
	(void)other;
}

irc::Server& irc::Server::operator=(const Server& other)
{
	if (this != &other) {
	}
	return *this;
}


int setSocketNonBlocking(int socket)
{
	if (fcntl(socket, F_SETFL, O_NONBLOCK) == -1) {
		perror("Error setting socket to non-blocking mode");
		return -1;
	}
	return 0;
}


void irc::Server::initServer(void)
{
	// Create socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}

	// // Set socket options
	// int opt = 1;
	// if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
	// 	perror("Error setting socket options");
	// 	exit(EXIT_FAILURE);
	// }

	// Set up server address struct
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(_port);

	// Bind socket
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("Error binding socket");
		exit(EXIT_FAILURE);
	}

	// Listen for incoming connections
	if (listen(serverSocket, BACKLOG) == -1) {
		perror("Error listening for connections");
		exit(EXIT_FAILURE);
	}

	setSocketNonBlocking(serverSocket);

	this->_socket = serverSocket;
}

void irc::Server::cleanServer(void)
{
	for (size_t i = 0; i < this->_users.size(); i++) {

		close(this->_users[i].getSocket());
	}
	close(this->_socket);
}


// use server socket to accept new incoming client connection
int irc::Server::acceptClient(void)
{
	sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientSocket = accept(_socket, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientSocket == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
				// No new clients
				return (0);
			} else {
				perror("Error accepting new connections");
				return (1);
			}
	}
	std::cout << "new connection! slayyy queen" << std::endl;
	setSocketNonBlocking(clientSocket);

	// add client to client vector
	_users.push_back(clientSocket);
	return (0);
}



// go through the list of each client socket (connected to us), and get incoming data if any
int irc::Server::collectClientData(void)
{
	for (size_t i = 0; i < _users.size(); i++) {

		irc::User& user = _users[i];

		// Receive data from this client
		#define RECV_BUFFER_SIZE 3000
		char buffer[RECV_BUFFER_SIZE];
		ssize_t bytesRead;
		while ((bytesRead = recv(user.getSocket(), buffer, sizeof(buffer), 0)) > 0) {

			// Process the received data
			buffer[bytesRead] = '\0';
			std::cout << "data received: " << buffer;
		}

		if (bytesRead == 0) {
			// Connection closed by the client
			// we remove the user from the user vector
			std::cerr << "Client closed the connection." << std::endl;
			_users.erase(_users.begin() + i);
			return (0);
		}
		else if (bytesRead == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				// No data available at the moment (non-blocking mode)
				// Handle this condition if needed, or continue processing
				return (0);
			} else {
				perror("Error receiving data");
				return (1);
			}
		}
	}

	return (0);
}



void irc::Server::startServer(void)
{
	initServer();

	while (true) {
		acceptClient();
		// std::cout << _users.size() << std::endl;
		collectClientData();
		usleep(1000);
	}

	cleanServer();
	return ;
}
