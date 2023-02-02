#ifndef UDATA_HPP
# define UDATA_HPP

#include <string>

struct Udata
{
	uintptr_t	sock_fd;
	std::string	msg;
};

#endif