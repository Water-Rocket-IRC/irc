#pragma once

#include <iostream>
#include <sys/event.h>

struct ChatUser 
{
	ChatUser(void);
	std::string	nickname_;
	std::string username_;
	std::string mode_;
	std::string unused_;
	std::string realname_;

	uintptr_t		client_sock_; //legacy variable, 곧 kevent로 대체해야 함
	int				mod;
	bool	operator==(const ChatUser& obj) const { return this->client_sock_ == obj.client_sock_; };
	bool	operator!=(const ChatUser& obj) const { return this->client_sock_ != obj.client_sock_; };
	bool	operator>(const ChatUser& obj) const { return this->client_sock_ > obj.client_sock_; };
	bool	operator<(const ChatUser& obj) const { return this->client_sock_ < obj.client_sock_; };
	bool	is_user_has_nick(void) const;

	//그 외 기타등등
};
