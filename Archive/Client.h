#pragma once

#include <queue>
#include <memory>
#include <string>
#include <map>
#include <set>
#include <functional>
#include <cstring>
#include <sstream>
#include <iostream>

#include "Asio.h"

#include "Message.h"


class Server;
class Channel;


class Client : public std::enable_shared_from_this<Client>
{
private:
	asio::ip::tcp::socket socket;
	//chat_room& room_;
	std::queue<Message> write_msgs_;

	char readBuffer[512]; // RFC 1459, max 512 byte
	std::size_t readBufferLength = 0;


	Server& server;

//	bool readCancelled = false;




public:
	std::string nick, lowerCaseNick, login, realName, host;
	std::set<Channel*> channels;


	Client(asio::ip::tcp::socket&& socket, std::string&& host, Server& server);

	void partAllChannels();

	~Client()
	{
		std::cout << "Chat session destructed" << std::endl;
	}

	void start()
	{
		doRead();
	}


	void setNick(std::string nick);

	const std::string& getLowerCaseNick() const
	{
		return lowerCaseNick;
	}


	//void cancelRead();


	template <typename... Args>
	void writeMessageFromThisClient(Message& out, const char* command, const Args... args) const
	{
		*out += ':';
		*out += nick;
		*out += '!';
		*out += login;
		*out += '@';
		*out += host;
		*out += ' ';
		*out += command;
		*out += ' ';

		writeMessageArgs(out, args...);

		*out += '\r';
		*out += '\n';
	}

	template <typename... Args>
	Message createMessageFromThisClient(const char* command, const Args&... args) const
	{
		auto msg = createEmptyMessage();
		writeMessageFromThisClient(msg, command, args...);
		return msg;
	}



	void send(Message msg);

private:



	void handleMessageFromClient(char* command, std::size_t length);



	void doRead();



	void doWrite();

};



//static std::map<std::string, chat_session::CommandHandler> chat_session::commandHandlers =

