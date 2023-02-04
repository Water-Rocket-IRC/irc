#include "ChatUser.hpp"

ChatUser::ChatUser(void)
{
	bzero(this, sizeof(*this));
}

bool ChatUser::is_user_has_nick(void) const
{
	return this->nickname_ != "" ? true : false;
}
