#ifndef USER_HPP
# define USER_HPP

#include <iostream>
#include <sys/event.h>

struct user 
{
	user(void);
	std::string	nickname_;
	std::string username_;
	std::string mode_;
	std::string unused_;
	std::string realname_;

	uintptr_t		client_sock_; //legacy variable, 곧 kevent로 대체해야 함
	int				mod;
	bool	operator==(const user& obj) const { return this->client_sock_ == obj.client_sock_; };
	bool	operator!=(const user& obj) const { return this->client_sock_ != obj.client_sock_; };
	bool	operator>(const user& obj) const { return this->client_sock_ > obj.client_sock_; };
	bool	operator<(const user& obj) const { return this->client_sock_ < obj.client_sock_; };
	bool	is_user_has_nick(void) const;
	//그 외 기타등등
};

user::user(void)
{
	bzero(this, sizeof(*this));
}

bool user::is_user_has_nick(void) const
{
	return this->nickname_ != "" ? true : false;
}
#endif