/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:16:15 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/13 15:06:03 by akhellad         ###   ########.fr       */
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
    void setUserName(const std::string& username);
    std::string getNickName() const;
    void setNickname(const std::string& nickname);
    std::string getRealName() const;
    void setRealName(const std::string& realname);
    std::string getHostName() const;
    void setHostName(const std::string& hostname);
    int get_port() const;
    const std::set<Channel*>& getChannels() const;

    void leave();

    void sendMessage(const std::string& message);

    bool getAuthentication() const;
    void setAuthentication(bool auth);

private:
    int socket_fd;
    std::string hostName;
    std::string userName;
    std::string nickName;
    std::string realName;
    int port;
    std::set<Channel*> channels;
	bool authenticated;
};

#endif // CLIENT_H


