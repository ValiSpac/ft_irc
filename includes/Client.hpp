/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:16:15 by akhellad          #+#    #+#             */
/*   Updated: 2023/11/10 09:40:28 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <netinet/in.h>

class Client {
public:
    Client(int socket_fd, int port, const std::string& hostname);
    ~Client();

    int getSocket() const;
    std::string get_hostname() const;
    int get_port() const;

    void leave(); // Vous devez définir cette fonction en fonction de vos besoins

private:
    int socket_fd;
    std::string hostname;
    int port;
};

#endif // CLIENT_H


