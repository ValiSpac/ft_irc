/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akhellad <akhellad@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/09 15:26:02 by akhellad          #+#    #+#             */
/*   Updated: 2024/01/22 11:48:30 by akhellad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include <iostream>
#include <exception>

void putError(const std::string& str);
bool strHasWhitespace(const std::string& str);
bool strIsNumeric(const std::string& str);

int main(int argc, char **argv) {

	if (argc != 3)
		return putError("Usage: ./ircserv <port> <password>"), 1;
	std::string port = argv[1];
	std::string password = argv[2];
	if (!strIsNumeric(port))
		return putError("Error: port must be numeric"), 1;
	if (strHasWhitespace(password) || password == "")
		return putError("Error: invalid password"), 1;

    try {
        Server server(port, password);
        server.start();
    } catch (const std::exception &e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

void putError(const std::string& str) {
	std::cerr << str << std::endl;
}

bool strHasWhitespace(const std::string& str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (std::isspace(static_cast<unsigned char>(*it))) {
			return true;
		}
	}
	return false;
}

bool strIsNumeric(const std::string& str) {
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
		if (!std::isdigit(*it)) {
			return false;
		}
	}
	return true;
}
