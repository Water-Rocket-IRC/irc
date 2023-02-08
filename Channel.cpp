#include "Channel.hpp"
#include "Udata.hpp"


// void	DebugshowUsers(std::vector<User>& target) {
// 	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
// 	for (std::vector<User>::iterator it = target.begin(); it != target.end(); ++it) {
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

std::string&	Channel::get_access(void) { return access_; }
void			Channel::set_access(const std::string& access) { access_ = access; }

void			Channel::change_nick(User& usr, std::string new_nick)
{
	std::vector<User>::iterator it;
	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->nickname_ == usr.nickname_)
		{
			usr.nickname_ = new_nick;
			break ;
		}
	}
}

std::vector<User>	Channel::sort_users(void)
{
	std::vector<User>	ret;

	ret.push_back(connectors_.at(0));
	if (connectors_.size() == 1)
		return ret;
	for (std::vector<User>::iterator it = connectors_.begin()
		 ; it != connectors_.end(); ++it)
	{
		std::vector<User>::iterator ret_it = ret.begin();
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

std::string		Channel::get_user_list_str(void)
{
	std::string					ret;
	std::vector<User> sort = sort_users();
	for (std::vector<User>::iterator it = sort.begin(); it != sort.end(); ++it)
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
Udata Channel::send_all(User& sender, User& target, std::string msg, int remocon)
{
	Udata			ret;
	std::vector<User>::iterator it;

	// if (is_user(sender) == false)
	// {
	// 	// Sender::error_message(sender.client_sock_, "")
	// }
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
				packet.second += Sender::mode_make_operator_message(sender, "Channel", *it);;
				break ;
			case PRIV:
				if (sender == *it)
				{
					packet.first = sender.client_sock_;
					packet.second = "";
				}
				else
				{
					packet = Sender::privmsg_channel_message(sender, *it, msg, this->get_name());
				}
				break ;
			case KICK:
				packet = Sender::kick_message(sender, *it, target.nickname_, this->get_name(), msg);
				break ;
			case QUIT:
				packet = (sender == *it) ? Sender::quit_leaver_message(sender, msg)
										 : Sender::quit_member_message(sender, *it, msg);
				break ;
			case NOTICE:
				if (sender == *it)
				{
					packet.first = sender.client_sock_;
					packet.second = "";
				}
				else
				{
					packet = Sender::notice_channel_message(sender, *it, msg, this->get_name());
				}
				break ;
			case WALL:
				if (sender == *it)
				{
					continue ;
				}
				packet = Sender::wall_message(sender, this->get_host(), this->get_name(), msg);
			case TOPIC:
				packet = Sender::topic_message(sender, *it, this->get_name(), msg);
				break ;
			case NICK:
				// if (sender == *it)
				// {
				// 	continue ;
				// }
				packet = Sender::nick_well_message(sender, *it, msg);
				break ;
			case WHO:
				packet = Sender::who_joiner_352_message(sender, this->get_name());
				break;
		}
		ret.insert(packet);
	}
	return ret;
}

std::string&	Channel::get_name(void)
{
	return this->name_;
}

/// @brief 있으면1, 없으면 0
bool Channel::is_user(User& usr)
{
	std::vector<User>::iterator it;

	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->client_sock_ == usr.client_sock_)
			return 1;
	}
	return 0;
}

void Channel::add_user(User& joiner)
{
	connectors_.push_back(joiner);
}

void Channel::set_host()
{
	this->host_ = connectors_[0];
}

void Channel::set_channel_name(std::string& chan_name)
{
	this->name_ = chan_name;
}

void Channel::set_topic(std::string& topic)
{
	this->topic_ = topic;
}

std::vector<User>& Channel::get_users()
{
	return this->connectors_;
}
void	Channel::delete_user(User& usr)
{
	std::vector<User>::iterator it = std::find(this->connectors_.begin(), \
	this->connectors_.end(), usr);
	std::size_t size = std::distance(this->connectors_.begin(), it);
	this->connectors_.erase(this->connectors_.begin() + size);
}
bool Channel::operator==(const Channel& t) const
{
	return (this->name_ == t.name_);
}
User& Channel::get_host()
{
	return this->host_;
}

std::string Channel::get_info()
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
