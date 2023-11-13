/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 14:47:10 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/13 11:58:09 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

Channel::Channel(const std::string& name)
    : name(name), inviteOnly(false), topicOperatorOnly(false), userLimit(-1) {
        serverName = "ircserv";
}

void Channel::debugPrintMembers() const {
    std::cout << "Membres du canal " << name << ":" << std::endl;
    for (std::set<Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (*it != NULL) {
            std::cout << " - " << (*it)->getNickName() << std::endl;
        }
    }

    std::cout << "Opérateurs du canal:" << std::endl;
    for (std::set<Client*>::const_iterator it = operators.begin(); it != operators.end(); ++it) {
        if (*it != NULL) {
            std::cout << " - " << (*it)->getNickName() << std::endl;
        }
    }
}

void Channel::setTopicOperatorOnly(bool value) {topicOperatorOnly = value;}

bool Channel::isTopicOperatorOnly() const {return topicOperatorOnly;}

void Channel::addMember(Client* client) {
    if (client == NULL) {
        return; // Ne pas ajouter si le client est NULL
    }
    members.insert(client);
    if (members.size() == 1) {
        operators.insert(client); // Ajouter comme opérateur si c'est le premier membre
    }
}

void Channel::handleTopicCommand(Client* setter, const std::string& newTopic) {
    if (isTopicOperatorOnly() && !isOperator(setter)) {
        std::string errorMessage = ":" + serverName + " 482 " + setter->getNickName() + " " + name
                                 + " :You're not channel operator\r\n";
        setter->sendMessage(errorMessage);
    } else {
        setTopic(newTopic);
        std::string topicMessage = ":" + setter->getNickName() + "!" + setter->getUserName()
                               + "@" + setter->getHostName() + " TOPIC " + name + " :" + newTopic + "\r\n";
        broadcastPrivateMessage(topicMessage, NULL);
    }
}

void Channel::removeMember(Client* client) {
    members.erase(client);
    operators.erase(client);
}

bool Channel::isInviteOnly() {return inviteOnly;}

bool Channel::isOperator(Client* client)
{
    std::set<Client*>::iterator it = operators.find(client);
    if (it != operators.end())
        return true;
    else
        return false;
}

void Channel::setOperator(std::string& target,Client* setter)
{
    std::set<Client*>::iterator memberIt = members.end();
    for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if ((*it)->getNickName() == target) {
            memberIt = it;
            break;
        }
    }
    std::set<Client*>::iterator operatorIt = operators.end();
    for (std::set<Client*>::iterator it = operators.begin(); it != operators.end(); ++it) {
        if ((*it)->getNickName() == target) {
            operatorIt = it;
            break;
        }
    }
    if (memberIt != members.end())
    {
        operators.insert(*memberIt);
        std::string opMessage = ":" + setter->getNickName() + " MODE " + name + " +o " + target + "\r\n";
        broadcastPrivateMessage(opMessage, NULL);
    }
    else if (operatorIt != operators.end())
    {
        std::string errorMessage = ":" + setter->getNickName() + " 482 " + name
                                 + " :User is already an operator\r\n";
        setter->sendMessage(errorMessage);
    }
    else
    {
        std::string errorMessage = ":" + setter->getNickName() + " 401 " + name
                                 + " :No such nick/channel\r\n";
        setter->sendMessage(errorMessage);
    }
}

void Channel::setTopic(const std::string& topic) {this->topic = topic;}

const std::string Channel::getName() const {return name;}

const std::set<Client*>& Channel::getMembers() const {return members;}

const std::set<Client*>& Channel::getOperators() const {return operators;}

int Channel::getUserLimit() {return userLimit;}

const std::string Channel::getTopic() const {return topic;}

void Channel::broadcastPrivateMessage(const std::string& message, const Client* sender) {
    for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if (*it != sender) {
            (*it)->sendMessage(message + "\r\n");
        }
    }
}

void Channel::inviteClient(Client* client) {inviteList.insert(client);}

void Channel::setKey(const std::string& key) {channelKey = key;}

void Channel::setUserLimit(int userLimit)
{
    if(userLimit > 0)
    {
        this->userLimit = userLimit;
    }
}

void Channel::setMode(Client* setter,std::istringstream& iss)
{
    if (!isOperator(setter)) {
        std::cerr << "Error: " << setter->getNickName() << " is not a channel operator." << std::endl;
        // Envoyer un message d'erreur au client
        setter->sendMessage(":" + serverName + " 482 " + setter->getNickName() + " " + name + " :You're not channel operator\r\n");
        return;
    }
    std::string sflag, options;
    iss >> sflag;
    if (sflag.empty()) {
        std::string modeString = getModes(); // Assurez-vous que cette méthode existe et renvoie les modes actuels du canal
        std::string modeMessage = ":" + serverName + " 324 " + setter->getNickName() + " " + name + " " + modeString + "\r\n";
        setter->sendMessage(modeMessage);
        return;
    }
    if (sflag.size() < 2 || (sflag[0] != '+' && sflag[0] != '-')) {
        // Format de drapeau invalide
        std::string errorMessage = ":" + serverName + " 472 " + setter->getNickName() + " " + name + " :Unknown mode flag\r\n";
        setter->sendMessage(errorMessage);
        return;
    }
    char flag = sflag[1];
    iss >> options;
        switch (flag) {
            case 'i':
                inviteOnly = !inviteOnly;
                break;
            case 't':
                topicOperatorOnly = !topicOperatorOnly;
                break;
            case 'k':
                if (channelKey.empty()) {
                    setKey(options);
                } else {
                    channelKey.clear();
                }
                break;
            case 'o':
                    setOperator(options, setter);
                    break;
            case 'l':
                if (options == "")
                    setUserLimit(-1);
                else
                    setUserLimit(std::atoi(options.c_str()));
                break;
            default:
            std::string errorMessage = ":" + setter->getNickName() + " 472 " + name
                                     + " :Unknown mode flag\r\n";
            setter->sendMessage(errorMessage);
            return;
    }
    std::string is = iss.str();
    std::string modeMessage = ":" + setter->getNickName() + " MODE " + name + " " + sflag + (options.empty() ? "" : " " + options) + "\r\n";
    broadcastPrivateMessage(modeMessage, NULL);
}

bool Channel::isMember(Client* client) const {
    return members.find(client) != members.end();
}

bool Channel::isInInvitList(Client* client) const {
    return inviteList.find(client) != inviteList.end();
}

bool Channel::isChannelFull()
{
    if (getUserLimit() == -1)
        return false;
    else if (getUserLimit() > (int)getMembers().size())
        return false;
    else
        return true;
}

std::string Channel::getModes() const {
    std::string modeString;
    if (inviteOnly) modeString += "i";
    if (topicOperatorOnly) modeString += "t";
    if (!channelKey.empty()) modeString += "k";
    // Ajouter d'autres flags de mode ici
    return modeString.empty() ? "+" : "+" + modeString;
}