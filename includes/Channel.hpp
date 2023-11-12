/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/10 11:26:05 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/12 20:21:34 by akhellad         ###   ########.fr       */
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
    const std::set<Client*>& getMembers() const;
    const std::set<Client*>& getOperators() const;
    const std::string getTopic() const;
    bool isInviteOnly();

    void setTopic(const std::string& topic);
    void setUserLimit(int userLimit);
    void setKey(const std::string& key);
    void setOperator(std::string& target, Client *setter);
    void setTopicOperatorOnly(bool value);

    void inviteClient(Client* client);
    bool isTopicOperatorOnly() const;
    bool isOperator(Client* client);
    void removeMember(Client* client);
    void broadcastPrivateMessage(const std::string& message, const Client* sender);
    void setMode(Client* setter,std::istringstream& iss);
    void handleTopicCommand(Client* setter, const std::string& newTopic);

    void debugPrintMembers() const;
private:
    std::string name;
    std::set<Client*> members;
    std::set<Client*> operators;
    std::set<Client*> inviteList;
    std::string topic;
    bool inviteOnly;
    bool topicOperatorOnly;
    std::string channelKey;
    int userLimit;
};

#endif // CHANNEL_H
