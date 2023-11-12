/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vpac <vpac@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 14:47:10 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/12 16:07:03 by vpac             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

Channel::Channel(const std::string& name)
    : name(name), inviteOnly(false), topicOperatorOnly(false), userLimit(-1) {
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
    if (isOperator(setter) || !isTopicOperatorOnly())
    {
        setTopic(newTopic);

        std::string topicMessage = ":" + setter->getNickName() + " TOPIC " + name + " :" + newTopic + "\r\n";
        broadcastPrivateMessage(topicMessage, NULL);
    }
    else
    {
        std::string errorMessage = ":" + setter->getNickName() + " 482 " + name
                                 + " :You're not allowed to set the topic for this channel\r\n";
        setter->sendMessage(errorMessage);
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

void Channel::setOperator(std::string& target)
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

const std::string Channel::getTopic() const {return topic;}

void Channel::broadcastPrivateMessage(const std::string& message, const Client* sender) {
    for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if (*it != sender) {
            (*it)->sendMessage(message + "/r/n");
        }
    }
}

void Channel::inviteClient(Client* client) {inviteList.insert(client);}

void Channel::setKey(const std::string& key) {channelKey = key;}

void Channel::setUserLimit(int userLimit)
{
    this->userLimit = userLimit;
}

void Channel::setMode(Client* setter,std::istringstream& iss)
{
    if (!isOperator(setter))
    {
        std::cerr << "Error: " << setter->getNickName() << " is not a channel operator." << std::endl;
        return;
    }
    std::string sflag, options;
    iss >> sflag;
    iss >> options;
    if (iss.fail())
        options = "";
    char flag = sflag[1];
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
                    setOperator(options);
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
    std::string modeMessage = ":" + setter->getNickName() + " MODE " + name + " " + is + "\r\n";
    broadcastPrivateMessage(modeMessage, NULL);
}
