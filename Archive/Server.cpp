#include "Server.h"
#include "Util.h"

using namespace std;

Server::Server(asio::io_service& io_service, unsigned short port)
:
	acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port), true),
	clientSocket(io_service)
{
	accept();
}



bool Server::handleCommandNick(Client& client, size_t argc, const char* argv[])
{
	if (argc != 1)
		return false;

	string nick = argv[0];

	if (nick.empty() || getClientByNick(nick))
		return false;

	removeClient(client);
	client.setNick(nick);
	addClient(client);

	cout << "New nickname " << client.nick << endl;

	return true;
}


bool Server::handleCommandJoin(Client& client, size_t argc, const char* argv[])
{
	if (argc != 1)
		return false;

	string channelName = argv[0];



	// Doesn't work with emplace for some reason
	// Channel& channel = channels.emplace(channelName, channelName, *this).first->second;
//		Channel& channel = channels.emplace(channelName, Channel(channelName, *this)).first->second;
	Channel& channel = channels.emplace(piecewise_construct,
			forward_as_tuple(channelName),
			forward_as_tuple(channelName, *this)).first->second;
//		Channel& channel = channels.insert(pair<const string, Channel>(
//				channelName, Channel(channelName, *this))).first->second;


	channel.addClient(client);

	return true;
}




Client* Server::getClientByNick(string nick)
{
	toLowerCase(nick);
	auto clientIterator = clients.find(nick);

	if (clientIterator != clients.end())
	{
		return &(*clientIterator->second);
	}
	else
		return nullptr;
}


Channel* Server::getChannelByName(const string& name)
{
	auto channelIterator = channels.find(name);

	if (channelIterator != channels.end())
	{
		return &channelIterator->second;
	}
	else
		return nullptr;
}





bool Server::handleCommandPart(Client& client, size_t argc, const char* argv[])
{
	if (argc < 1 || argc > 2)
		return false;

	string channelName = argv[0];

	Channel* channel = getChannelByName(channelName);

	if (channel)
	{
		channel->removeClient(client);
	}

	return true;
}



bool Server::handleCommandUser(Client& client, size_t argc, const char* argv[])
{
	if (argc != 4)
		return false;

	client.login = argv[0];
	client.realName = argv[3];

	auto welcome = createEmptyMessage();
	writeMessageFromServer(welcome, client, ServerReply::RPL_WELCOME , "Welcome to the Internet Relay Network " + client.nick + '!' + client.login + '@' + client.host);
	writeMessageFromServer(welcome, client, ServerReply::RPL_YOURHOST, "Your host is " + name + ", running version 0.01");
	writeMessageFromServer(welcome, client, ServerReply::RPL_CREATED , "This server was created " __DATE__);
	writeMessageFromServer(welcome, client, ServerReply::RPL_MYINFO  , name + " 0.01  ");
	client.send(move(welcome));

	return true;
}


bool Server::handleClientCommand(Client& client, const char* command, size_t argc, const char* argv[])
{
	switch (command[0])
	{
	case 'I':
		if (command[1] == 'S' && command[2] == 'O' && command[3] == 'N' && command[4] == '\0')
			return handleCommandIsOn(client, argc, argv);
		break;
	case 'J':
		if (command[1] == 'O' && command[2] == 'I' && command[3] == 'N' && command[4] == '\0')
			return handleCommandJoin(client, argc, argv);
		break;
	case 'M':
		if (command[1] == 'O' && command[2] == 'D' && command[3] == 'E' && command[4] == '\0')
			return handleCommandMode(client, argc, argv);
		break;
	case 'N':
		if (command[1] == 'A' && command[2] == 'M' && command[3] == 'E' && command[4] == 'S' && command[5] == '\0')
			return handleCommandNames(client, argc, argv);
		if (command[1] == 'I' && command[2] == 'C' && command[3] == 'K' && command[4] == '\0')
			return handleCommandNick(client, argc, argv);
		if (command[1] == 'O' && command[2] == 'T' && command[3] == 'I' && command[4] == 'C' && command[5] == 'E' && command[6] == '\0')
			return handleCommandPrivMsg(client, argc, argv, command);
		break;
	case 'P':
		if (command[1] == 'A' && command[2] == 'R' && command[3] == 'T' && command[4] == '\0')
			return handleCommandPart(client, argc, argv);
		if (command[1] == 'R' && command[2] == 'I' && command[3] == 'V' && command[4] == 'M' && command[5] == 'S' && command[6] == 'G' && command[7] == '\0')
			return handleCommandPrivMsg(client, argc, argv, command);
		break;
	case 'Q':
		if (command[1] == 'U' && command[2] == 'I' && command[3] == 'T' && command[4] == '\0')
			return handleCommandQuit(client, argc, argv);
		break;
	case 'U':
		if (command[1] == 'S' && command[2] == 'E' && command[3] == 'R' && command[4] == '\0')
			return handleCommandUser(client, argc, argv);
		break;
	case 'W':
		if (command[1] == 'H' && command[2] == 'O' && command[3] == '\0')
			return handleCommandWho(client, argc, argv);
		break;

	}

	sendMessageFromServer(client, ServerReply::ERR_UNKNOWNCOMMAND, command, "Unknown command");

	return false;
}


void Server::removeClient(const Client& client)
{
	clients.erase(client.getLowerCaseNick());
}

void Server::addClient(Client& client)
{
	clients.insert(make_pair(client.getLowerCaseNick(), client.shared_from_this()));
}


void Server::accept()
{
	acceptor.async_accept(clientSocket, clientRemoteEndpoint, [this](error_code error)
	{
		if (!error)
		{
			string host = clientRemoteEndpoint.address().to_string();
			cout << "Client from " << host << " connected" << endl;
			auto client = make_shared<Client>(move(clientSocket), move(host), *this); // Following the move, the moved-from object is in the same state as if constructed using the basic_stream_socket(io_service&) constructor.
			addClient(*client);
			client->start();
		}
		//else
		//	throw boost::system::system_error(error);

		accept();
	});
}


bool Server::handleCommandQuit(Client& client, size_t /*argc*/, const char* /*argv*/[])
{
	quitClient(client);
	return true;
}

bool Server::handleCommandMode(Client& client, size_t /*argc*/, const char* /*argv*/[])
{
	sendMessageFromServer(client, ServerReply::RPL_UMODEIS, "");
	return true;
}

bool Server::handleCommandNames(Client& client, size_t argc, const char* argv[])
{
	if (argc != 1)
		return false;

	Channel* channel = getChannelByName(argv[0]);
	if (channel)
		channel->sentNameReply(client);

	return true;
}

bool Server::handleCommandIsOn(Client& client, size_t argc, const char* argv[])
{
	if (argc < 1)
		return false;

	string response;

	for (size_t i = 0; i < argc; i++)
		if (getClientByNick(argv[i]))
		{
			response += argv[i];
			response += ' ';
		}

	sendMessageFromServer(client, ServerReply::RPL_ISON, response);

	return true;
}


bool Server::handleCommandWho(Client& client, size_t argc, const char* argv[])
{
	if (argc < 1)
		return false;

	Message message = createEmptyMessage();

	const char* name = argv[0];

	Channel* channel = getChannelByName(name);
	if (channel)
	{
		for (auto clientOnChannel : channel->getClients())
		writeMessageFromServer(message, client, ServerReply::RPL_WHOREPLY, name,
				clientOnChannel->login, clientOnChannel->host, this->name, clientOnChannel->nick,
				"H@", "0 " + clientOnChannel->realName);
	}
	writeMessageFromServer(message, client, ServerReply::RPL_ENDOFWHO, name, "End of /WHO list.");

	client.send(move(message));

	return true;
}

bool Server::handleCommandPrivMsg(Client& client, size_t argc, const char* argv[], const char* command)
{
	if (argc != 2)
		return false;

	const char* recipient = argv[0];
	const char* text = argv[1];

	Message message = client.createMessageFromThisClient(command, recipient, text);

	Channel* channel = getChannelByName(recipient);
	if (channel)
	{
		channel->broadcast(message, &client);
		return true;
	}

	Client* otherClient = getClientByNick(recipient);
	if (otherClient)
	{
		otherClient->send(move(message));
		return true;
	}

	sendMessageFromServer(client, ServerReply::ERR_NOSUCHNICK, recipient, "No such nick/channel");
	return false;
}


void Server::quitClient(Client& client)
{
	client.partAllChannels();
	removeClient(client);
}



