#pragma once

#include "User.hpp"
#include <vector>
#include <string>


namespace irc {

	class Server
	{
		public:
			Server(int port, std::string password);
			~Server();
			Server(const Server &other);
			Server& operator=(const Server &other);

			void startServer(void);

		private:

			void initServer(void);
			void cleanServer(void);
			int acceptClient(void);
			int collectClientData(void);

			int _socket;
			int _port;
			int _backlog;
			int _bufferSize;
			std::string _password;
			std::vector<irc::User> _users;

	};
}
