#include "User.hpp"

User::User(void)
{
	bzero(this, sizeof(*this));
}

bool User::is_user_has_nick(void) const
{
	return this->nickname_.size() ? true : false;
}

void	User::input_user(const std::string& username, const std::string& mode
					, const std::string& unused, const std::string& realname)
{
	username_ = username;
	mode_ = mode;
	unused_ = unused;
	realname_ = realname;
}
