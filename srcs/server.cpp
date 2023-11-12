/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:16:39 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/12 20:36:20 by akhellad         ###   ########.fr       */
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
#include <sstream>

#define MAX_CONNECTIONS 10

Server::Server(const std::string &port, const std::string &pass)
    : _port(port), _pass(pass), _running(true)
{
    _sock = create_socket();
    serverName = "ircserv";
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
    sprintf(message, "%s:%d has connected.", client->getHostName().c_str(), client->get_port());
    log(message);
}

void Server::on_client_disconnect(int fd) {
    try {
        Client* client = _clients.at(fd);

        // Loguer le message de déconnexion
        char message[1000];
        sprintf(message, "%s:%d has disconnected!", client->getHostName().c_str(), client->get_port());
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
    nbytes = read(fd, buffer, sizeof(buffer) - 1);

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

        parseClientCommand(fd, buffer);
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

void Server::handleJoinCommand(Client* client, const std::string& channelName) {
    if (client->getNickName().empty() || client->getNickName().substr(0, 5) == "Guest") {
        std::string errorMsg = ":" + serverName + " 431 :No nickname given\r\n";
        client->sendMessage(errorMsg);
        return;
    }
    Channel* channel;

    // Vérifier si le canal existe déjà
    std::map<std::string, Channel*>::iterator it = channels.find(channelName);
    if (it != channels.end()) {
        channel = it->second;
    } else {
        channel = new Channel(channelName);
        channels[channelName] = channel;
    }
    channel->addMember(client);
    channel->debugPrintMembers();
    std::ostringstream response;
    response << ":" << client->getNickName() << "!" << client->getUserName()
             << "@" << client->getHostName() << " JOIN :" << channelName << "\r\n";

    client->sendMessage(response.str());

    // Envoyer le sujet du canal (si disponible)
    if (!channel->getTopic().empty()) {
        response.str(""); // Effacer le flux
        response << ":" + serverName + " 332 " << client->getNickName() << " " << channelName
                 << " :" << channel->getTopic() << "\r\n";
        client->sendMessage(response.str());
    }

    // Envoyer la liste des membres du canal
    response.str(""); // Effacer le flux
    response << ":" + serverName + " 353 " << client->getNickName() << " = " << channelName << " :" << "\r\n";
    const std::set<Client*>& members = channel->getMembers();
    for (std::set<Client*>::const_iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
        if (*memberIt != NULL) {
            response << (*memberIt)->getNickName() << " ";
        }
    }
    response << "\r\n:" + serverName + " 366 " << client->getNickName() << " " << channelName
             << " :End of /NAMES list.\r\n";
    client->sendMessage(response.str());
}

void Server::handleNickCommand(Client* client, const std::string& nickname) {
    // Vérifier si le pseudonyme est déjà utilisé par un autre client
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickName() == nickname) {
            client->sendMessage(":" + serverName + " 433 * " + nickname + " :Nickname is already in use\r\n");
            return;
        }
    }

    // Mettre à jour le pseudonyme du client
    client->setNickname(nickname);

    // Envoyer une confirmation au client
    client->sendMessage(":" + serverName + " NICK :" + nickname + "\r\n");
}

void Server::handlePrivMsgCommand(Client* sender, const std::string& target, const std::string& message) {
    if (target[0] == '#') {  // La cible un canal
        Channel* channel = getChannelByName(target);
        if (channel) {
            std::string fullMessage = ":" + sender->getNickName() + "!" + sender->getUserName()
                                  + "@" + sender->getHostName() + " PRIVMSG " + target + " :" + message + "\r\n";

            const std::set<Client*>& members = channel->getMembers();
            for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
                if ((*it) != sender)
                {
                    (*it)->sendMessage(fullMessage);
                }
            }

            // Feedback à l'expéditeur
        } else {
            sender->sendMessage(":" + serverName + " 403 " + sender->getNickName() + " " + target + " :No such channel\r\n");
        }
    } else {  // La cible est un utilisateur
        Client* recipient = getClientByNickname(target);
        if (recipient) {
            recipient->sendMessage(":" + sender->getNickName() + "!" + sender->getUserName()
                                   + "@" + sender->getHostName() + " PRIVMSG " + target + " :" + message + "\r\n");
        } else {
            sender->sendMessage(":" + serverName + " 401 " + sender->getNickName() + " " + target + " :No such nick/channel\r\n");
        }
    }
}

void Server::handleKickCommand(Client* sender, const std::string& channelName, const std::string& targetNickname) {
    Channel* channel = getChannelByName(channelName);
    if (channel) {
        Client* target = getClientByNickname(targetNickname);
        if (target) {
            if (channel->isOperator(sender))
            {
                channel->removeMember(target);

                std::ostringstream response;
                response << ":" << serverName << " KICK " << channelName << " " << targetNickname
                         << " :You have been kicked by " << sender->getNickName() << "\r\n";
                target->sendMessage(response.str());

                response.str("");
                response << ":" << serverName << " 441 " << sender->getNickName() << " " << channelName
                         << " " << targetNickname << " :Kicked by " << sender->getNickName() << "\r\n";
                sender->sendMessage(response.str());
            }
            else
            {
                sender->sendMessage(":" + serverName + " 482 " + sender->getNickName() + " " + channelName
                                   + " :You're not channel operator\r\n");
            }
        }
        else
        {
            sender->sendMessage(":" + serverName + " 401 " + sender->getNickName() + " " + targetNickname
                               + " :No such nick/channel\r\n");
        }
    }
    else
    {
        sender->sendMessage(":" + serverName + " 403 " + sender->getNickName() + " " + channelName
                           + " :No such channel\r\n");
    }
}

void Server::handleModeCommand(Client* setter, const std::string& channelName,std::istringstream& mode) {
    Channel* channel = getChannelByName(channelName);
    if (channel) {
        channel->setMode(setter, mode);
    } else {
        setter->sendMessage(":" + serverName + " 403 " + setter->getNickName() + " " + channelName
                           + " :No such channel\r\n");
    }
}

void Server::handleTopicCommand(Client* client, const std::string& channelName, const std::string& newTopic) {
    Channel* channel = getChannelByName(channelName);
    if (channel) {
        channel->handleTopicCommand(client, newTopic);
    } else {
        client->sendMessage(":" + serverName + " 403 " + client->getNickName() + " " + channelName
                           + " :No such channel\r\n");
    }
}

void Server::handleInviteCommand(Client* sender, const std::string& channelName, const std::string& targetNickname)
{
    Channel* channel = getChannelByName(channelName);
    if (channel) {
        if (channel->isOperator(sender)) {
            Client* target = getClientByNickname(targetNickname);
            if (target) {
                channel->inviteClient(target);
                sender->sendMessage(":" + serverName + " 341 " + sender->getNickName() + " " + targetNickname
                                     + " " + channelName + " :Inviting " + targetNickname + "\r\n");
                target->sendMessage(":" + serverName + " INVITE " + targetNickname + " " + channelName + "\r\n");
            } else {
                sender->sendMessage(":" + serverName + " 401 " + sender->getNickName() + " " + targetNickname
                                   + " :No such nick/channel\r\n");
            }
        } else {
            sender->sendMessage(":" + serverName + " 482 " + sender->getNickName() + " " + channelName
                               + " :You're not channel operator\r\n");
        }
    } else {
        sender->sendMessage(":" + serverName + " 403 " + sender->getNickName() + " " + channelName
                           + " :No such channel\r\n");
    }
}

void Server::parseClientCommand(int fd, const std::string& command) {
    Client* client = getClientByFD(fd);
    if (!client) {
        std::cerr << "Client not found for fd: " << fd << std::endl;
        return;
    }
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "JOIN") {
        std::string channelName;
        iss >> channelName;  // Supposer que la syntaxe est "JOIN #channelname"
        handleJoinCommand(client, channelName); // Assurez-vous d'avoir une méthode pour trouver le Client par fd
    }
    if (cmd == "NICK") {
        std::string nickname;
        iss >> nickname;
        handleNickCommand(client, nickname);
    }
    if (cmd == "PRIVMSG") {
        std::string target, message;
        iss >> target;
        std::getline(iss, message);
        if (!message.empty() && message[0] == ':') {
            message = message.substr(1);  // Enlever le ':' initial
        }
        handlePrivMsgCommand(client, target, message);
    }
    if (cmd == "KICK") {
        std::string channelName, targetNickname;
        iss >> channelName >> targetNickname;
        handleKickCommand(client, channelName, targetNickname);
    }
    if (cmd == "TOPIC")
    {
    std::string channelName, newTopic;
    iss >> channelName;
    std::getline(iss, newTopic);
    if (!newTopic.empty() && newTopic[0] == ':') {
        newTopic = newTopic.substr(1);
    }
    handleTopicCommand(client, channelName, newTopic);
    }
    if (cmd == "INVITE") {
        std::string channelName, targetNickname;
        iss >> targetNickname >> channelName;
        handleInviteCommand(client, channelName, targetNickname);
    }
    if (cmd == "MODE"){
        std::string channelName;
        iss >> channelName;
        handleModeCommand(client, channelName, iss);
    }
    // Ajouter ici la gestion d'autres commandes...
}

Client* Server::getClientByFD(int fd) {
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        return it->second;
    } else {
        return NULL;
    }
}

Channel* Server::getChannelByName(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    if (it != channels.end()) {
        return it->second;
    } else {
        return NULL;  // Canal non trouvé
    }
}

Client* Server::getClientByNickname(const std::string& nickname) {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickName() == nickname) {
            return it->second;
        }
    }
    return NULL;
}
