/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:16:39 by akhellad          #+#    #+#             */
/*   Updated: 2024/01/22 14:58:32 by akhellad         ###   ########.fr       */
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
#include <string>

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
    pfd.revents = 0;//YOU FORGOT TO FUCKING INITILIZE THE REVENTS
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

        char message[1000];
        sprintf(message, "%s:%d has disconnected!", client->getHostName().c_str(), client->get_port());
        log(message);
        const std::set<Channel*>& channels = client->getChannels();
        for (std::set<Channel*>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
            Channel* channel = *it;
            std::string quitMessage = ":" + client->getNickName() + "!" + client->getUserName()
                                      + "@" + client->getHostName() + " QUIT :Disconnected\r\n";
            channel->broadcastPrivateMessage(quitMessage, client);
            channel->removeMember(client);
        }
        close(fd);
        _clients.erase(fd);
        for (size_t i = 0; i < _pfds.size(); ++i) {
            if (_pfds[i].fd == fd) {
                _pfds.erase(_pfds.begin() + i);
                break;
            }
        }
        delete client;
    }
    catch (const std::exception &e) {
        std::cerr << "Error while disconnecting a client: " << e.what() << std::endl;
    }
}

void Server::on_client_message(int fd) {
    static std::map<int, std::string> incompleteCommands;
    char buffer[1024];
    int nbytes;
    nbytes = read(fd, buffer, sizeof(buffer) - 1);
    if (nbytes <= 0) {
        if (nbytes == 0) {
            std::cout << "Client with fd " << fd << " disconnected." << std::endl;
        } else {
            std::cerr << "Error reading from client with fd " << fd << std::endl;
        }
        on_client_disconnect(fd);
    } else {
        buffer[nbytes] = '\0';
        std::string data = incompleteCommands[fd] + std::string(buffer);
        size_t pos = 0;
        while ((pos = data.find('\n')) != std::string::npos) {
            std::string command = data.substr(0, pos);
            std::cout << "Client with fd " << fd << " sent : " << command << std::endl;
            parseClientCommand(fd, command);
            data.erase(0, pos + 1);
        }
        incompleteCommands[fd] = data;
    }
}

void Server::log(const std::string& message) {
    std::time_t now = std::time(NULL);
    char buf[100] = {0};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    std::cout << "[" << buf << "] " << message << std::endl;
}

void Server::handleJoinCommand(Client* client, const std::string& channelName, const std::string& key) {
    if (client->getNickName().empty() || client->getNickName().substr(0, 5) == "Guest") {
        std::string errorMsg = ":" + serverName + " 431 :No nickname given\r\n";
        client->sendMessage(errorMsg);
        return;
    }
    Channel* channel = NULL;
    std::map<std::string, Channel*>::iterator it = channels.find(channelName);
    if (it != channels.end() && !it->second->isInviteOnly() && !it->second->isChannelFull())
    {
        channel = it->second;
    }
    else if(it != channels.end() && it->second->isInviteOnly() && it->second->isInInvitList(client))
    {
        channel = it->second;
    }
    else if (it != channels.end() && it->second->isInviteOnly() && !it->second->isInInvitList(client))
    {
        std::string errorMsg = ":" + serverName + " 473 " + client->getNickName() + " " + channelName + " :Cannot join channel (+i)\r\n";
        client->sendMessage(errorMsg);
        return ;
    }
    else if (it != channels.end() && it->second->isChannelFull() ){
        std::string errorMsg = ":" + serverName + " 471 " + client->getNickName() + " " + channelName + " :Cannot join channel (+l)\r\n";
        client->sendMessage(errorMsg);
        return ;
    }
    else {
        channel = new Channel(channelName);
        channels[channelName] = channel;
    }
    std::cout << channel->getKey() << std::endl;
    std::cout << key << std::endl;
    if (channel->hasKey()) {
        if (key != channel->getKey()) {
            std::string errorMsg = ":" + serverName + " 475 " + client->getNickName() + " " + channelName + " :Cannot join channel (+k)\r\n";
            client->sendMessage(errorMsg);
            return;
        }
    }
    channel->addMember(client);
    std::string joinMessage = ":" + client->getNickName() + "!" + client->getUserName()
                              + "@" + client->getHostName() + " JOIN :" + channelName + "\r\n";
    const std::set<Client*>& members = channel->getMembers();
    for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        Client* member = *it;
        if (member != NULL) {
            member->sendMessage(joinMessage);
        }
    }
    std::ostringstream response;
    if (!channel->getTopic().empty()) {
        response << ":" + serverName + " 332 " << client->getNickName() << " " << channelName
                 << " :" << channel->getTopic() << "\r\n";
        client->sendMessage(response.str());
        response.str("");
    }
    response << ":" << serverName << " 353 " << client->getNickName() << " = " << channelName << " :";
    for (std::set<Client*>::const_iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
        if (*memberIt != NULL) {
            response << (*memberIt)->getNickName() << " ";
        }
    }
    response << "\r\n:" << serverName << " 366 " << client->getNickName() << " " << channelName << " :End of NAMES list\r\n";
    client->sendMessage(response.str());
}

void Server::handleNickCommand(Client* client, const std::string& nickname) {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickName() == nickname) {
            client->sendMessage(":" + serverName + " 433 * " + nickname + " :Nickname is already in use\r\n");
            return;
        }
    }
    client->setNickname(nickname);
    client->sendMessage(":" + serverName + " NICK :" + nickname + "\r\n");
}

void Server::handlePassCommand(Client* client, const std::string& password) {
	if (password == this->_pass)
    {
		client->setAuthentication(true);
        std::string test = ":" + serverName + " 001 " + client->getNickName() + " :" + "Welcome " + client->getNickName() + "\r\n";
        client->sendMessage(test);
    }
	else
		client->sendMessage("ERROR :Invalid password\r\n");
}

void Server::handlePrivMsgCommand(Client* sender, const std::string& target, const std::string& message) {
    if (target[0] == '#') {
        Channel* channel = getChannelByName(target);
        if (channel && channel->isMember(sender)) {
            std::string fullMessage = ":" + sender->getNickName() + "!" + sender->getUserName()
                                  + "@" + sender->getHostName() + " PRIVMSG " + target + " :" + message + "\r\n";

            const std::set<Client*>& members = channel->getMembers();
            for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
                if ((*it) != sender)
                {
                    (*it)->sendMessage(fullMessage);
                }
            }
        } else if (channel && !channel->isMember(sender)){
            sender->sendMessage(":" + serverName + " 404 " + sender->getNickName() + " " + target + " :Cannot send to channel\r\n");
        }
         else if (channel) {
            sender->sendMessage(":" + serverName + " 403 " + sender->getNickName() + " " + target + " :No such channel\r\n");
        }
    } else {
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
            if (channel->isOperator(sender)) {
                channel->removeMember(target);
                std::ostringstream responseToTarget;
                responseToTarget << ":" << sender->getNickName() << "!" << sender->getUserName()
                                 << "@" << sender->getHostName() << " KICK " << channelName << " " << targetNickname
                                 << " :Kicked by " << sender->getNickName() << "\r\n";
                target->sendMessage(responseToTarget.str());
                std::ostringstream responseToChannel;
                responseToChannel << ":" << serverName << " KICK " << channelName << " " << targetNickname
                                  << " :Kicked by " << sender->getNickName() << "\r\n";
                const std::set<Client*>& members = channel->getMembers();
                for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
                    if (*it != target) {
                        (*it)->sendMessage(responseToChannel.str());
                    }
                }
            } else {
                sender->sendMessage(":" + serverName + " 482 " + sender->getNickName() + " " + channelName
                                   + " :You're not channel operator\r\n");
            }
        } else {
            sender->sendMessage(":" + serverName + " 401 " + sender->getNickName() + " " + targetNickname
                               + " :No such nick/channel\r\n");
        }
    } else {
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

void Server::handleWhoCommand(Client* client, const std::string& channelName) {
    Channel* channel = getChannelByName(channelName);
    if (channel) {
        const std::set<Client*>& members = channel->getMembers();
        const std::set<Client*>& operators = channel->getOperators();

        for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
            Client* member = *it;
            std::ostringstream response;
            std::string userPrefix = operators.find(member) != operators.end() ? "@" : "";
            response << ":" << serverName << " 352 " << client->getNickName() << " " << channelName << " "
                     << member->getUserName() << " " << member->getHostName() << " " << serverName << " "
                     << userPrefix << member->getNickName() << " H :0\r\n";
            client->sendMessage(response.str());
        }
        client->sendMessage(":" + serverName + " 315 " + client->getNickName() + " " + channelName + " :End of WHO list\r\n");
    } else {
        client->sendMessage(":" + serverName + " 403 " + client->getNickName() + " " + channelName + " :No such channel\r\n");
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

void Server::handlePartCommand(Client* client, const std::string& channelName) {
    std::string realChannelName = channelName.substr(0, channelName.find(' '));
    Channel* channel = getChannelByName(realChannelName);

    if (channel && channel->isMember(client)) {
        std::string partMessage = ":" + client->getNickName() + "!" + client->getUserName()
                                  + "@" + client->getHostName() + " PART " + realChannelName + "\r\n";

        channel->broadcastPrivateMessage(partMessage, client);
        channel->removeMember(client);
        client->sendMessage(partMessage);
    } else if (channel) {
        client->sendMessage(":" + serverName + " 442 " + client->getNickName() + " " + realChannelName + " :You're not on that channel\r\n");
    } else {
        client->sendMessage(":" + serverName + " 403 " + client->getNickName() + " " + realChannelName + " :No such channel\r\n");
    }
}

void Server::handleQuitCommand(Client* client, const std::string& message)
{
    std::ostringstream response;
    response << ":" << client->getNickName() << "!" << client->getUserName()
             << "@" << client->getHostName() << " QUIT :" << message << "\r\n";
    client->sendMessage(response.str());
    on_client_disconnect(client->getSocket());
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

    if (cmd == "QUIT")
    {
        std::string message;
        std::getline(iss, message);
        if (!message.empty() && message[0] == ':') {
            message = message.substr(1);
        }
        handleQuitCommand(client, message);
        return ;
    }
    if (cmd == "NICK") {
        std::string nickname;
        iss >> nickname;
        handleNickCommand(client, nickname);
    }
    if (cmd == "USER") {
        std::string username;
        std::string hostname;
        std::string servername;
        std::string realname;
    	iss >> username >> hostname >> servername;
		std::getline(iss >> std::ws, realname);
		client->setUserName(username);
		client->setHostName(hostname);
		client->setRealName(realname);
    }
    if (cmd == "PASS") {
        std::string password;
        iss >> password;
        handlePassCommand(client, password);
    }
	if (client->getAuthentication() == false)
		return;

    if (cmd == "JOIN") {
        std::string channelName;
        iss >> channelName;
        std::string keyArgument;
        std::getline(iss, keyArgument);
        if (!keyArgument.empty() && keyArgument[0] == ' ') {
            keyArgument = keyArgument.substr(1);
        }
        handleJoinCommand(client, channelName, keyArgument);
    }
    if (cmd == "PRIVMSG") {
        std::string target, message;
        iss >> target;
        std::getline(iss, message);
        if (!message.empty() && message[0] == ':') {
            message = message.substr(1);
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
    if (cmd == "PART") {
        std::string channelName;
        iss >> channelName;
        if (!channelName.empty())
        {
            std::string channelName;
            iss >> channelName;
            size_t pos = channelName.find(' ');
            if (pos != std::string::npos) {
                channelName = channelName.substr(0, pos);
            }
        }
        handlePartCommand(client, channelName);
    }
    if (cmd == "LIST") {
        std::ostringstream response;
        response << ":" << serverName << " 321 " << client->getNickName() << " Channel :Users Name\r\n";
        client->sendMessage(response.str());
        for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
            response.str("");
            response << ":" << serverName << " 322 " << client->getNickName() << " " << it->first << " " << it->second->getMembers().size() << " :";
            for (std::set<Client*>::const_iterator it2 = it->second->getMembers().begin(); it2 != it->second->getMembers().end(); ++it2) {
                if (*it2 != NULL) {
                    response << (*it2)->getNickName() << " ";
                }
            }
            response << "\r\n";
            client->sendMessage(response.str());
        }
    }
    if (cmd == "WHO") {
		std::string target;
		iss >> target;
		handleWhoCommand(client, target);
    }

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
        return NULL;
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
