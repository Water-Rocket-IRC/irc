#include "ChatRoom.hpp"


// void	DebugshowUsers(std::vector<ChatUser>& target) {
// 	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
// 	for (std::vector<ChatUser>::iterator it = target.begin(); it != target.end(); ++it) {
// 		std::cout << (*it).nickname_ << " ➜ ";
// 	}
// 	std::cout << YELLOW << "(end)" << RESET << std::endl;
// }

// void	DebugshowChannels(std::vector<Chan>& target) {
// 	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
// 	for (std::vector<Chan>::iterator it = target.begin(); it != target.end(); ++it) {
// 		std::cout << (*it).get_name() << " ➜ ";
// 	}
// 	std::cout << YELLOW << "(end)" << RESET << std::endl;
// }

std::string&	ChatRoom::get_access(void) { return access_; }
void			ChatRoom::set_access(const std::string& access) { access_ = access; }

void			ChatRoom::change_nick(ChatUser& usr, std::string new_nick)
{
	std::vector<ChatUser>::iterator it;
	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->nickname_ == usr.nickname_)
		{
			usr.nickname_ = new_nick;
			break ;
		}
	}
}

std::vector<ChatUser>	ChatRoom::sort_users(void)
{
	std::vector<ChatUser>	ret;

	ret.push_back(connectors_.at(0));
	if (connectors_.size() == 1)
		return ret;
	for (std::vector<ChatUser>::iterator it = connectors_.begin()
		 ; it != connectors_.end(); ++it)
	{	
		std::vector<ChatUser>::iterator ret_it = ret.begin();
		for (; ret_it != ret.end(); ++ret_it)
		{
			if (*it > *ret_it)
			{
				continue ;
			}
			else if (*it < *ret_it || *it == *ret_it)
			{
				break ;
			}
		} 
		if (ret_it == ret.end() || *it != *ret_it)
			ret.insert(ret_it, *it);
	}
	return ret;
}

std::string		ChatRoom::get_user_list_str(void)
{
	std::string					ret;
	std::vector<ChatUser> sort = sort_users();
	for (std::vector<ChatUser>::iterator it = sort.begin(); it != sort.end(); ++it)
	{
		ret += *it == get_host() ? "@" + it->nickname_ : it->nickname_;
		ret += " ";
	}
	return ret;
}

/*
///@ brief 기본형 : 나 빼고 다 보냄, flag 1 -> PART
*/
//TODO : Sender가 event를 잘 리턴하는지 파악해야 함
Udata ChatRoom::send_all(ChatUser& sender, ChatUser& target, std::string msg, int remocon)
{
	Udata			ret;
	std::vector<ChatUser>::iterator it;

	if (is_user(sender) == false)
	{
		// Sender::error_message(sender.client_sock_, "")
	}
	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		Event packet;

		switch (remocon)
		{
			case JOIN:
				packet = Sender::join_message(sender, *it, this->get_name());
				break ;
			case PART:
				packet = Sender::part_message(sender, *it, this->get_name(), msg);
				break ;
			case PRIV:
				if (sender == *it)
				{
					continue ;
				}
				packet = Sender::privmsg_channel_message(sender, *it, msg, this->get_name());
				break ;
			case KICK:
				packet = Sender::kick_message(sender, *it, target.nickname_, this->get_name(), msg);
				break ;
			case QUIT:
				packet = Sender::quit_channel_message(sender, *it, msg);
				break ;
			case NOTICE:
				if (sender == *it)
				{
					continue ;
				}
				packet = Sender::notice_channel_message(sender, *it, msg, this->get_name());
				break ;
			case WALL:
				if (sender == *it)
				{
					continue ;
				}
				packet = Sender::wall_message(sender, this->get_host(), this->get_name(), msg);
			case TOPIC:
				//Sender::
				break ;
			case NICK:
				if (sender == *it)
				{
					continue ;
				}
				break ;
			case WHO:
				packet = Sender::who_joiner_352_message(sender, this->get_name());
				break;
		}
		ret.insert(packet);
	}
	return ret;
}

std::string&	ChatRoom::get_name(void)
{
	return this->name_;
}

/// @brief 있으면1, 없으면 0
bool ChatRoom::is_user(ChatUser& usr)
{
	std::vector<ChatUser>::iterator it;

	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->client_sock_ == usr.client_sock_)
			return 1;
	}
	return 0;
}

void ChatRoom::add_user(ChatUser& joiner)
{
	connectors_.push_back(joiner);
}
void ChatRoom::set_host()
{
	this->host_ = connectors_[0];
}
void ChatRoom::set_channel_name(std::string& chan_name)
{
	this->name_ = chan_name;
}

void ChatRoom::set_topic(std::string& topic)
{
	this->topic_ = topic;
}

std::vector<ChatUser>& ChatRoom::get_users()
{
	return this->connectors_;
}
void	ChatRoom::delete_user(ChatUser& usr)
{
	std::vector<ChatUser>::iterator it = std::find(this->connectors_.begin(), \
	this->connectors_.end(), usr);
	std::size_t size = std::distance(this->connectors_.begin(), it);
	this->connectors_.erase(this->connectors_.begin() + size);
}
bool ChatRoom::operator==(const ChatRoom& t) const
{
	return (this->name_ == t.name_);
}
ChatUser& ChatRoom::get_host()
{
	return this->host_;
}

std::string ChatRoom::get_info()
{
	std::string ret;
	std::stringstream index_ss;
	index_ss << connectors_.size();

	ret =	"Channel's name  : " + name_ + "\n" +
			"Channel's topic : " + topic_ + "\n" +
			"Connected users : " + index_ss.str() + "\n" +
			"Channel's host  : " + host_.nickname_;
	return ret;
} 
