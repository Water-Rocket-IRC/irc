#include <iostream>
#include <sys/event.h>

struct user 
{
	std::string	nickname_;
	std::string username_;
	std::string hostname_;
	std::string servername_;
	std::string realname_;

	struct kevent event;
	int			client_sock_; //legacy variable, 곧 kevent로 대체해야 함
	int			mod;
	bool	operator==(const user& obj) { return this->nickname_ == obj.nickname_; };
	//그 외 기타등등
};