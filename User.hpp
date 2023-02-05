#pragma once

#include <iostream>

	// User(const std::string& nickname, const std::)

struct User 
{
	std::string	nickname_;
	std::string username_;
	std::string mode_;
	std::string unused_;
	std::string realname_;
	uintptr_t	client_sock_;
	int			mod;


	User(void);
	bool	operator==(const User& obj) const { return this->client_sock_ == obj.client_sock_; };
	bool	operator!=(const User& obj) const { return this->client_sock_ != obj.client_sock_; };
	bool	operator>(const User& obj) const { return this->client_sock_ > obj.client_sock_; };
	bool	operator<(const User& obj) const { return this->client_sock_ < obj.client_sock_; };
	bool	is_user_has_nick(void) const;

	//그 외 기타등등
};
