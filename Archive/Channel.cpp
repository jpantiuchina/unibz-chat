#include "Channel.h"
#include "Server.h"


using namespace std;

void Channel::removeClient(Client& client)
{

	if (client.channels.erase(this))
	{
		broadcast(client.createMessageFromThisClient("PART", name));
		clients.erase(client.shared_from_this()); // Erasing this after broadcasting
	}
}




void Channel::addClient(Client& client)
{
	if (client.channels.count(this) == 0)
	{
		clients.insert(client.shared_from_this());
		client.channels.insert(this);
		broadcast(client.createMessageFromThisClient("JOIN", name));

		sentNameReply(client);

	}
}


void Channel::sentNameReply(Client& client)
{
	auto response = createEmptyMessage();

	string names;
	for (auto aclient : clients)
	{
		names += aclient->nick;
		names += ' ';
	}

	server.writeMessageFromServer(response, client, ServerReply::RPL_NAMREPLY, '=', name, names);
	server.writeMessageFromServer(response, client, ServerReply::RPL_ENDOFNAMES, name, "End of NAMES list");
	client.send(std::move(response));
}


void Channel::broadcast(const Message& msg, Client* exceptFor)
{
	for (auto client : clients)
		if (&(*client) != exceptFor)
			client->send(msg);
}

