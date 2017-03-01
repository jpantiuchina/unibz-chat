#pragma once

#include <memory>
#include <set>
#include <map>

#include "Asio.h"
#include "Channel.h"
#include "Client.h"
#include "ServerReply.h"
#include "Message.h"



class Server
{
private:
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket clientSocket;
	asio::ip::tcp::endpoint clientRemoteEndpoint;

	std::string name = "irc.unibz.it";

	std::map<const std::string, std::shared_ptr<Client>> clients;
	std::map<const std::string, Channel> channels;


public:
	Server(asio::io_service& io_service, unsigned short port);



	template <typename... Args>
	void writeMessageFromServer(Message& out, const Client& client, ServerReply reply, const Args&... args) const
	{
		int code = static_cast<int>(reply);
		*out += ':';
		*out += name;
		*out += ' ';
		*out += '0' + code / 100;
		*out += '0' + code / 10 % 10;
		*out += '0' + code % 10;
		*out += ' ';
		*out += client.nick;
		*out += ' ';
		writeMessageArgs(out, args...);
		*out += '\r';
		*out += '\n';
	}




	template <typename... Args>
	void sendMessageFromServer(Client& client, ServerReply reply, const Args&... args) const
	{
		auto msg = createEmptyMessage();
		writeMessageFromServer(msg, client, reply, args...);
		client.send(msg);
	}



	bool handleClientCommand(Client& client, const char* command, std::size_t argc, const char* argv[]);



	Client* getClientByNick(std::string nick);
	Channel* getChannelByName(const std::string& name);


	void quitClient(Client& client);


private:
	void accept();
	void removeClient(const Client& client);
	void addClient(Client& client);


	bool handleCommandNick(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandJoin(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandPart(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandUser(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandQuit(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandMode(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandNames(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandIsOn(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandWho(Client& client, std::size_t argc, const char* argv[]);

	bool handleCommandPrivMsg(Client& client, std::size_t argc, const char* argv[], const char* command);
};
