#pragma once

#include <memory>
#include <asio.hpp>

#include "ChatRoom.hpp"
#include "ChatSession.hpp"

class ServerSocket
{
private:
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket socket;
	chat_room room;

public:
	ServerSocket(asio::io_service& io_service, unsigned short port)
	:
		acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port), false),
		socket(io_service)
	{
		accept();
	}

private:
	void accept()
	{
		acceptor.async_accept(socket, [this](std::error_code error)
		{
			if (!error)
			{
				std::make_shared<chat_session>(std::move(socket), room)->start(); // Following the move, the moved-from object is in the same state as if constructed using the basic_stream_socket(io_service&) constructor.
			}
			//else
			//	throw boost::system::system_error(error);

			accept();
		});
	}

};