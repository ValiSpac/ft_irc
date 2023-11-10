/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 11:26:05 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/10 15:12:39 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <set>
#include "Client.hpp"

class Client;

class Channel {
public:
    Channel(const std::string& name);

    void addMember(Client* client);
    const std::string getName() const;
    const std::set<Client*> getMembers() const;
    const std::set<Client*> getOperators() const;
    const std::string getTopic() const;

    void setTopic(const std::string& topic);

    void removeMember(Client* client);
    void broadcastPrivateMessage(const std::string& message, const Client* sender);

private:
    std::string name;
    std::set<Client*> members;
    std::set<Client*> operators;
    std::string topic;
};

#endif // CHANNEL_H
