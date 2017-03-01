#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>


class Message
{
public:
	static const std::size_t HEADER_LENGTH   = 4;
	static const std::size_t MAX_BODY_LENGTH = 512;

private:
	char data[HEADER_LENGTH + MAX_BODY_LENGTH];
	std::size_t bodyLength = 0;

public:
	Message() { }

	const char* getData() const { return data; }
	      char* getData()       { return data; }

	const char* getBody() const { return data + HEADER_LENGTH; }
	      char* getBody()       { return data + HEADER_LENGTH; }

	std::size_t getBodyLength() const { return                 bodyLength; }
	std::size_t getDataLength() const { return HEADER_LENGTH + bodyLength; }

	void setBodyLength(std::size_t length)
	{
		bodyLength = std::min(length, MAX_BODY_LENGTH);
	}

	
	bool decode_header()
	{
		char header[HEADER_LENGTH + 1] = "";
		std::strncpy(header, data, HEADER_LENGTH);
		bodyLength = std::atoi(header);
		if (bodyLength > MAX_BODY_LENGTH)
		{
			bodyLength = 0;
			return false;
		}
		return true;
	}

	/*
	void encode_header()
	{
		char header[HEADER_LENGTH + 1] = "";
		std::sprintf(header, "%4d", bodyLength);
		std::memcpy(data, header, HEADER_LENGTH);
	}
	*/

};
