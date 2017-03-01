#pragma once

#include <set>
#include <deque>
#include <memory>

#include "Message.h"
#include "Client.h"
#include "ServerReply.h"

class Server;

class Channel
{
private:
	std::set<std::shared_ptr<Client>> clients;
	std::string name;
	Server& server;

public:
	Channel(const std::string name, Server& server)
	:
		name(std::move(name)),
		server(server)
	{
	}


	const std::set<std::shared_ptr<Client>> getClients() const
	{
		return clients;
	}


	void broadcast(const Message& msg, Client* exceptFor = nullptr);


	void addClient(Client& client);

	void removeClient(Client& client);


	void sentNameReply(Client& client);



	const std::string& getName()
	{
		return name;
	}

};
