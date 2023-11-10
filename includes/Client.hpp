/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:16:15 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/10 15:39:01 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <netinet/in.h>
#include <Channel.hpp>

class Channel;

class Client {
public:
    Client(int socket_fd, int port, const std::string& hostname);
    ~Client();

    int getSocket() const;
    std::string getUserName() const;
    std::string getNickName() const;
    void setNickname(const std::string& nickname);
    std::string getHostName() const;
    int get_port() const;

    void leave();

    void sendMessage(const std::string& message);

private:
    int socket_fd;
    std::string hostName;
    std::string userName;
    std::string nickName;
    int port;
    std::set<Channel*> channels;
};

#endif // CLIENT_H


