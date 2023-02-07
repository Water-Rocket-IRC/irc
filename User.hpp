#pragma once

#include <iostream>


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
	void	input_user(const std::string& username, const std::string& mode
						, const std::string& unused, const std::string& realname);
	bool	operator==(const User& obj) const { return this->client_sock_ == obj.client_sock_; };
	bool	operator!=(const User& obj) const { return this->client_sock_ != obj.client_sock_; };
	bool	operator>(const User& obj) const { return this->client_sock_ > obj.client_sock_; };
	bool	operator<(const User& obj) const { return this->client_sock_ < obj.client_sock_; };
	bool	is_user_has_nick(void) const;
};
