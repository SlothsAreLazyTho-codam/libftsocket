#pragma once

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

class Utils
{
	public:
		static const std::string addrintoipv4(in_addr addr);
};