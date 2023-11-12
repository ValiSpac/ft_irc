/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:16:12 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/12 20:35:06 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include "Client.hpp"
#include <cstdio>
#include <Channel.hpp>

class Server {
public:
    Server(const std::string &port, const std::string &pass);
    ~Server();
    void start();
    void parseClientCommand(int fd, const std::string& command);

    void handleJoinCommand(Client* client, const std::string& channelName);
    void handleNickCommand(Client* client, const std::string& nickname);
    void handlePrivMsgCommand(Client* sender, const std::string& target, const std::string& message);
    void handleKickCommand(Client* sender, const std::string& channelName, const std::string& targetNickname);
    void handleModeCommand(Client* setter, const std::string& channelName, std::istringstream& mode);
    void handleTopicCommand(Client* client, const std::string& channelName, const std::string& newTopic);
    void handleInviteCommand(Client* sender, const std::string& channelName, const std::string& targetNickname);

    Channel* getChannelByName(const std::string& name);
    Client* getClientByNickname(const std::string& nickname);
    Client* getClientByFD(int fd);
private:
    std::string _port;
    std::string _pass;
    int _sock;
    bool _running;
    std::vector<pollfd> _pfds;
    std::map<int, Client*> _clients;
    std::map<std::string, Channel*> channels;
    std::string serverName;

    int create_socket();
    void on_client_connect();
    void on_client_disconnect(int fd);
    void on_client_message(int fd);
    std::string read_message(int fd);
    void log(const std::string &message);
};

#endif

