#include "User.hpp"

User::User(void)
{
	bzero(this, sizeof(*this));
}

bool User::is_user_has_nick(void) const
{
	return this->nickname_ != "" ? true : false;
}
