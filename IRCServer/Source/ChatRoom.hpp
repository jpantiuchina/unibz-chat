#pragma once

#include <set>
#include <deque>
#include <memory>

#include "Message.hpp"

class chat_participant
{
public:
	virtual ~chat_participant() {}
	virtual void deliver(const Message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;



class chat_room
{
public:
	void join(chat_participant_ptr participant)
	{
		participants_.insert(participant);
		for (auto msg : recent_msgs_)
			participant->deliver(msg);
	}

	void leave(chat_participant_ptr participant)
	{
		participants_.erase(participant);
	}

	void deliver(const Message& msg)
	{
		recent_msgs_.push_back(msg);
		while (recent_msgs_.size() > max_recent_msgs)
			recent_msgs_.pop_front();

		for (auto participant : participants_)
			participant->deliver(msg);
	}

private:
	std::set<chat_participant_ptr> participants_;
	enum { max_recent_msgs = 100 };
	std::deque<Message> recent_msgs_;
};
