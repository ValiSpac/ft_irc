#pragma once

#include <vector>
#include <string>


namespace irc {

	class User
	{
		public:
			User(int socket);
			~User();
			User(const User &other);
			User& operator=(const User &other);

			int getSocket(void) const;
			char* getBuffer(void) const;

		private:

			int _socket;
			bool _isOperator;


	};
}

