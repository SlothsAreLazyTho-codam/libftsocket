#include "Utils.hpp"
#include <ostream>
#include <iostream>
#include <string>

const std::string Utils::addrintoipv4(in_addr addr)
{
		std::string	str;

		str += std::to_string(addr.s_addr >> 0 & 0xFF) + ".";
		str += std::to_string(addr.s_addr >> 8 & 0xFF) + ".";
		str += std::to_string(addr.s_addr >> 16 & 0xFF) + ".";
		str += std::to_string(addr.s_addr >> 24 & 0xFF);

		return (str);
}