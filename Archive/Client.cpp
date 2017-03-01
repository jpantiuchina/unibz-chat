#include "Client.h"
#include "Channel.h"
#include "Server.h"
#include "Util.h"

using namespace std;




Client::Client(asio::ip::tcp::socket&& socket, string&& host, Server& server)
:
	socket(move(socket)),
	server(server),
	host(move(host))
{
	static int clientCount = 0;
	setNick("unregistered" + to_string(clientCount));
	clientCount++;
	cout << "Chat session constructed" << endl;
}


void Client::handleMessageFromClient(char* command, size_t length)
{
	if (length <= 2) // RFC 1459, empty messages are ignored
		return;

//		cout << length;

	command[length - 1] = '\0';
	command[length - 2] = '\0';

	cout << "< " << command << endl;

	auto ch  = command;

	if (*ch == ':') // Ignoring prefix, if any
	{
		while (*ch && *ch != ' ') // prefix
			ch++;
		while (*ch == ' ') // skipping spaces
			ch++;
	}




	const char* argv[16]; // RFC 1459: ...command and command parameters (of which there may be up to 15).
	size_t argc = 0;

	while (*ch && *ch != ':' && argc < sizeof(argv) / sizeof(argv[0]))
	{
		argv[argc] = ch;
		argc++;

		while (*ch && *ch != ' ') // read argument
			ch++;

		if (*ch) // According to the standard the should be extra space in the and, but LimeChat sends one only after the command
		{
			*ch = '\0'; // add terminator
			ch++;
		}

		while (*ch == ' ') // skip spaces
			ch++;
	}


	if (*ch == ':' && argc < sizeof(argv) / sizeof(argv[0]))
	{
		ch++;

		argv[argc] = ch;
		argc++;

		while (*ch) // might be skipped, but like that it checks for NUL inside message are enabled
			ch++;
	}

//		cout << reinterpret_cast<size_t>(ch) << endl;
//		cout << reinterpret_cast<size_t>(command + length - 2) << endl;

	if (ch == command + length - 2 && argc) // parsed correctly
	{
		server.handleClientCommand(*this, argv[0], argc - 1, argv + 1);
	}
	else
	{
		cout << "Error parsing message" << endl;
	}



}







void Client::doRead()
{
	auto self(shared_from_this());


	socket.async_read_some(asio::buffer(readBuffer + readBufferLength, sizeof(readBuffer) - readBufferLength),
	[this, self](error_code error, size_t length)
	{
		//cout << "read, status " << error << ", length " << length << endl;

		if (error || !length)
		{
			cout << "Chat session - header decode error" << endl;
			server.quitClient(*this);
			return;
		}


		char* messageStart = readBuffer;
		char* messageNewPartStart = readBuffer + readBufferLength;
		char* bufferEnd = messageNewPartStart + length;
//			size_t commandNewPartLength = length;

/*
		cout << "--" << endl;
		for (size_t i = 0; i < length; i++)
			cout << (int) (readBuffer + readBufferLength)[i] << " (" << (readBuffer + readBufferLength)[i] << ")" << endl;
		cout << "--" << endl;
*/

		while (true)
		{
			char* messageTerminalChar = static_cast<char*>(memchr(messageNewPartStart, '\n',
					bufferEnd - messageNewPartStart));


			if (messageTerminalChar) // full command found in buffer
			{
				size_t messageLength = messageTerminalChar - messageStart + 1;

//					cout << "command found, length " << commandLength << endl;



				handleMessageFromClient(messageStart, messageLength);


//					if ()
//					{
//						cout << "command(s) finished, buffer empty" << endl;
//						readBufferLength = 0;
//						doRead();
//						return;
//					}


				// moving to the next command
//					commandNewPartLength -= commandLength - (commandNewPartStart - commandStart);
				messageStart += messageLength;
				messageNewPartStart = messageStart;


			}
			else
			{
//					if (commandNewPartLength) // if a non-complete command left in the buffer
//				{
				readBufferLength = bufferEnd - messageStart;

					memmove(readBuffer, messageStart, readBufferLength);
	//			}
		//		else
			//	readBufferLength += length;
//					cout << "readBufferLength = " << readBufferLength << endl;

				doRead();
				return;
			}
		}
	});




}




void Client::doWrite()
{
	auto self(shared_from_this());

	cout << "> " << write_msgs_.front()->substr(0, write_msgs_.front()->length() - 2) << endl;

	asio::async_write(socket, asio::buffer(write_msgs_.front()->data(), write_msgs_.front()->length()),
	[this, self](error_code ec, size_t /*length*/)
	{
		if (!ec)
		{
			write_msgs_.pop();
			if (!write_msgs_.empty())
			{
				doWrite();
			}
		}
		else
		{
			server.quitClient(*this);
		}
	});
}


void Client::send(Message msg)
{
	bool write_in_progress = !write_msgs_.empty();
	write_msgs_.push(move(msg));
	if (!write_in_progress)
	{
		doWrite();
	}
}



//void Client::cancelRead()
//{
//	readCancelled = true;
//}


void Client::setNick(string nick)
{
	this->nick = nick;
	this->lowerCaseNick = move(nick);
	toLowerCase(this->lowerCaseNick);
}


void Client::partAllChannels()
{
	while (channels.size())
		(*channels.begin())->removeClient(*this);
}
