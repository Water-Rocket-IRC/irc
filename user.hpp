#include <iostream>
#include <sys/event.h>

struct user 
{
	std::string	nickname_;
	std::string username_;
	std::string hostname_;
	std::string servername_;
	std::string realname_;

	uintptr_t		client_sock_; //legacy variable, 곧 kevent로 대체해야 함
	int				mod;
	bool	operator==(const user& obj) { return this->client_sock_ == obj.client_sock_; };
	//그 외 기타등등
};