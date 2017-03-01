//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <asio.hpp>

#include "ServerSocket.hpp"

using namespace std;



int main(int argc, char* argv[])
{
	try
	{
		if (argc < 2)
		{
			cerr << "Usage: chat_server <port> [<port> ...]" << endl;
			return EXIT_FAILURE;
		}

		asio::io_service io_service;

		list<ServerSocket> servers;
		for (int i = 1; i < argc; i++)
		{
			int port = atoi(argv[i]);
			cout << "Listening on port " << port << endl;
			servers.emplace_back(io_service, port);
		}

		cout << "Server running..." << endl;
		io_service.run();
	}
	catch (exception& e)
	{
		cerr << "Exception: " << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
