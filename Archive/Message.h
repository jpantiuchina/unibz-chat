#pragma once

#include <string>
#include <memory>


typedef std::shared_ptr<std::string> Message;


Message createEmptyMessage();


template<typename T>
void writeMessageArgs(Message& out, const T& arg)
{
	*out += ':';
	*out += arg;
}


template<typename First, typename... Rest>
void writeMessageArgs(Message& out, const First& first, const Rest&... rest)
{
	*out += first;
	*out += ' ';
	writeMessageArgs(out, rest...);
}





