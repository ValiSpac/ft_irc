/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 14:47:10 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/10 22:43:20 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include <algorithm>
#include <iostream>

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

Channel::Channel(const std::string& name) : name(name) {}

void Channel::addMember(Client* client) {
    if (client == NULL) {
        return; // Ne pas ajouter si le client est NULL
    }
    members.insert(client);
    if (members.size() == 1) {
        operators.insert(client); // Ajouter comme opérateur si c'est le premier membre
    }
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
