/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/22 13:04:09 by akhellad          #+#    #+#             */
/*   Updated: 2024/01/22 13:04:11 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>

Client::Client(int socket_fd, int port, const std::string& hostName)
    : socket_fd(socket_fd), hostName(hostName), port(port), authenticated(false) {
    std::ostringstream ss;
    ss << "Guest" << socket_fd;
    nickName = ss.str();
}

Client::~Client() {
    close(socket_fd);
}

int Client::getSocket() const {
    return socket_fd;
}

std::string Client::getHostName() const {
    return hostName;
}

void Client::setHostName(const std::string& hostname) {
	this->hostName = hostname;
}

std::string Client::getUserName() const {
    return userName;
}

void Client::setUserName(const std::string& username) {
    this->userName = username;
}

std::string Client::getNickName() const {
    return nickName;
}

void Client::setNickname(const std::string& nickname) {
    this->nickName = nickname;
}


std::string Client::getRealName() const {
	return realName;
}

void Client::setRealName(const std::string& realname) {
    this->realName = realname;
}

bool Client::getAuthentication() const {
	return authenticated;
}

void Client::setAuthentication(bool auth) {
	authenticated = auth;
}

const std::set<Channel*>& Client::getChannels() const {
    return channels;
}

int Client::get_port() const {
    return port;
}

void Client::leave() {
    for (std::set<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        (*it)->removeMember(this);
    }
}

void Client::sendMessage(const std::string& message) {
    const char* msg = message.c_str();
    size_t len = strlen(msg);

    ssize_t bytes_sent = send(socket_fd, msg, len, 0);
    if (bytes_sent < 0) {
        std::cerr << "Erreur lors de l'envoi du message au client." << std::endl;
    }
}

