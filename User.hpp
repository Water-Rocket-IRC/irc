#pragma once

#include <iostream>

enum e_flags { F_PASS = 0b100, F_NICK = 0b010, F_USER = 0b001};

struct User
{
	std::string	nickname_;
	std::string username_;
	std::string mode_;
	std::string unused_;
	std::string realname_;
	char		flag_;
	uintptr_t	client_sock_;

	User(void);
	void	input_user(const std::string& username, const std::string& mode
						, const std::string& unused, const std::string& realname);
	bool	operator==(const User& obj) const { return this->client_sock_ == obj.client_sock_; };
	bool	operator!=(const User& obj) const { return this->client_sock_ != obj.client_sock_; };
	bool	operator>(const User& obj) const { return this->client_sock_ > obj.client_sock_; };
	bool	operator<(const User& obj) const { return this->client_sock_ < obj.client_sock_; };
	bool	is_user_has_nick(void) const;
};
