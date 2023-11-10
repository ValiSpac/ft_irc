/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 14:47:10 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/10 16:55:51 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel(const std::string& name) : name(name) {}

void Channel::addMember(Client* client) {
    if (members.empty()) {
        operators.insert(client);
    }
    members.insert(client);
}

void Channel::removeMember(Client* client) {
    members.erase(client);
    operators.erase(client);
}

void Channel::setTopic(const std::string& topic) { this->topic = topic;}

const std::string Channel::getTopic() const {return topic;}

const std::string Channel::getName() const {return name;}

const std::set<Client*>& Channel::getMembers() const {return members;}

const std::set<Client*>& Channel::getOperators() const {return operators;}

void Channel::broadcastPrivateMessage(const std::string& message, const Client* sender) {
    for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it) {
        if (*it != sender) {
            (*it)->sendMessage(message + "/r/n");
        }
    }
}
