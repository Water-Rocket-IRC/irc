#include <iostream>
#include <sys/event.h>

struct user
{
	std::string	nickname_;
	std::string username_;
	std::string hostname_;
	std::string realname_;

	struct kevent	event;

	// udata.ident =  event.ident (udata socket fd에 넣어주면 됌)

	
	// int				client_sock_; //legacy variable, 곧 kevent로 대체해야 함
	int				mod;
	//그 외 기타등등
};