/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:16:39 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/10 10:08:52 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <ctime>
#include <cstddef>
#include <algorithm>

#define MAX_CONNECTIONS 10

Server::Server(const std::string &port, const std::string &pass) 
    : _port(port), _pass(pass), _running(true)
{
    _sock = create_socket();
}

Server::~Server() 
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        delete it->second;
    }
    close(_sock);
}

int Server::create_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        throw std::runtime_error("Error while opening a socket!");
    }

    int optval = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
        throw std::runtime_error("Error while setting socket options!");
    }

    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(_port.c_str()));

    if (bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Error while binding a socket!");
    }

    if (listen(sock_fd, MAX_CONNECTIONS) < 0) {
        throw std::runtime_error("Error while listening on a socket!");
    }

    return sock_fd;
}

void Server::start() 
{
    pollfd srv;
    srv.fd = _sock;
    srv.events = POLLIN;
    _pfds.push_back(srv);

    while (_running) {
        if (poll(&_pfds[0], _pfds.size(), -1) < 0) {
            throw std::runtime_error("Error while polling from fd!");
        }

        for (size_t i = 0; i < _pfds.size(); i++) {
            if (_pfds[i].revents == 0) {
                continue;
            }

            if (_pfds[i].revents & (POLLHUP | POLLERR)) {
                on_client_disconnect(_pfds[i].fd);
                continue;
            }

            if (_pfds[i].revents & POLLIN) {
                if (_pfds[i].fd == _sock) {
                    on_client_connect();
                } else {
                    on_client_message(_pfds[i].fd);
                }
            }
        }
    }
}

void Server::on_client_connect() 
{
    int fd;
    sockaddr_in addr;
    socklen_t size = sizeof(addr);

    fd = accept(_sock, (sockaddr *)&addr, &size);
    if (fd < 0) {
        throw std::runtime_error("Error while accepting a new client!");
    }

    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    _pfds.push_back(pfd);

    char hostname[NI_MAXHOST];
    int res = getnameinfo((struct sockaddr *)&addr, sizeof(addr), hostname, NI_MAXHOST, NULL, 0, NI_NUMERICSERV);
    if (res != 0) {
        throw std::runtime_error("Error while getting hostname on a new client!");
    }

    Client* client = new Client(fd, ntohs(addr.sin_port), std::string(hostname));
    _clients.insert(std::make_pair(fd, client));

    char message[1000];
    sprintf(message, "%s:%d has connected.", client->get_hostname().c_str(), client->get_port());
    log(message);
}

void Server::on_client_disconnect(int fd) {
    try {
        Client* client = _clients.at(fd);

        // Loguer le message de déconnexion
        char message[1000];
        sprintf(message, "%s:%d has disconnected!", client->get_hostname().c_str(), client->get_port());
        log(message);

        // Fermer le socket du client
        close(fd);

        // Supprimer le client de la liste
        _clients.erase(fd);

        // Trouver et supprimer le fd correspondant dans _pfds
        for (size_t i = 0; i < _pfds.size(); ++i) {
            if (_pfds[i].fd == fd) {
                _pfds.erase(_pfds.begin() + i);
                break;
            }
        }

        // Libérer la mémoire
        delete client;
    }
    catch (const std::exception &e) {
        std::cerr << "Error while disconnecting a client: " << e.what() << std::endl;
    }
}

void Server::on_client_message(int fd) {
    char buffer[1024];  // Buffer pour lire les données
    int nbytes;

    // Essayer de lire les données du client
    nbytes = read(fd, buffer, sizeof(buffer));

    if (nbytes <= 0) {
        // Si read renvoie 0, le client a fermé la connexion
        // Si read renvoie -1, une erreur s'est produite (qui pourrait être EWOULDBLOCK si en mode non bloquant)
        if (nbytes == 0) {
            std::cout << "Client with fd " << fd << " disconnected." << std::endl;
        } else {
            std::cerr << "Error reading from client with fd " << fd << std::endl;
        }

        on_client_disconnect(fd);
    } else {
        // Traiter les données reçues
        buffer[nbytes] = '\0';
        std::cout << "Received message from client with fd " << fd << ": " << buffer << std::endl;

        // Ici, vous pourriez ajouter un traitement supplémentaire pour les données reçues
    }
}


std::string Server::read_message(int ) 
{
    // Similar to your existing read_message function
    // Add additional logic as per your requirement
    return (NULL);
}

void Server::log(const std::string& message) {
    // Obtenir l'heure et la date actuelles
    std::time_t now = std::time(NULL);
    char buf[100] = {0};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    // Écrire le message dans la console
    std::cout << "[" << buf << "] " << message << std::endl;
}

