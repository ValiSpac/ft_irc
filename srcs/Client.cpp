#include "../includes/Client.hpp"
#include <unistd.h>
#include <cstring>

Client::Client(int socket_fd, int port, const std::string& hostname) 
    : socket_fd(socket_fd), hostname(hostname), port(port) {}

Client::~Client() {
    close(socket_fd);
}

int Client::getSocket() const {
    return socket_fd;
}

std::string Client::get_hostname() const {
    return hostname;
}

int Client::get_port() const {
    return port;
}

void Client::leave() {
    // Ajoutez ici la logique pour gérer le départ du client, comme le nettoyage des ressources, etc.
}


