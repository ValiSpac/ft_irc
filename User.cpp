#include "User.hpp"

irc::User::User(int socket) :
	_socket(socket),
	_isOperator(false)
{
}

irc::User::~User()
{
}

irc::User::User(const User& other)
{
	*this = other;
}

irc::User& irc::User::operator=(const User& other)
{
	if (this != &other) {
		_socket = other._socket;
		_isOperator = other._isOperator;
	}
	return *this;
}

int irc::User::getSocket(void) const {
	return (_socket);
}

