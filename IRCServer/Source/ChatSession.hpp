#pragma once

#include <queue>
#include <memory>
#include <asio.hpp>

#include "ChatRoom.hpp"


class chat_session : public chat_participant, public std::enable_shared_from_this<chat_session>
{
	asio::ip::tcp::socket socket_;
	chat_room& room_;
	Message read_msg_;
	std::queue<Message> write_msgs_;

public:
	chat_session(asio::ip::tcp::socket socket, chat_room& room)
	:
		socket_(std::move(socket)),
		room_(room)
	{
		std::cout << "Chat session constructed" << std::endl;
	}


	virtual ~chat_session() override
	{
		std::cout << "Chat session destructed" << std::endl;
	}

	void start()
	{
		room_.join(shared_from_this());
		do_read_header();
	}

	void deliver(const Message& msg) override
	{
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push(msg);
		if (!write_in_progress)
		{
			do_write();
		}
	}

private:
	void do_read_header()
	{
		auto self(shared_from_this());

		asio::async_read(socket_, asio::buffer(read_msg_.getData(), Message::HEADER_LENGTH), [this, self](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec && read_msg_.decode_header())
			{
//				std::cout << "removind from room";
//				room_.leave(shared_from_this());
				do_read_body();
			}
			else
			{
				std::cout << "Chat session - header decode error" << std::endl;
				room_.leave(shared_from_this());
			}
		});
	}

	void do_read_body()
	{
		auto self(shared_from_this());
		asio::async_read(socket_, asio::buffer(read_msg_.getBody(), read_msg_.getBodyLength()), [this, self](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				room_.deliver(read_msg_);
				do_read_header();
			}
			else
			{
				room_.leave(shared_from_this());
			}
		});
	}

	void do_write()
	{
		auto self(shared_from_this());
		asio::async_write(socket_,
								 asio::buffer(write_msgs_.front().getData(),
								 write_msgs_.front().getDataLength()),
								 [this, self](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				write_msgs_.pop();
				if (!write_msgs_.empty())
				{
					do_write();
				}
			}
			else
			{
				room_.leave(shared_from_this());
			}
		});
	}


};