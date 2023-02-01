#pragma once

#include "Udata.hpp"
#include "Sender.hpp"
#include <vector>
#include <algorithm>
#include <sstream>


enum e_send_switch { JOIN, PART, PRIV, KICK, QUIT, NOTICE, TOPIC };
/*
 * JOIN : 나를 포함해서 모두에게 보여주는 메세지(메세지가 다름)
 * PART : 나를 포함해서 모두에게 보여주는 메세지(메세지가 다름)
 * PRIV : 채팅방에서 그냥 작성하는 메세지 (나한테 보여지는 건 client가 해주는 것)
 */

class Chan
{
	private:
		std::string			name_;
		std::string			topic_;
		int					error_;
		user				host_;				// operator
		std::vector<user>	connectors_;		// connectors.at(0) => operator

	public:
		std::string		get_info(void);

		std::string	get_name();
		user		get_host();
		void 		set_host();
		void		set_topic(std::string& topic);
		bool 		is_user(user& usr);
		void 		add_user(user& joiner);
		void 		set_channel_name(std::string& chan_name);
		void		delete_user(user& usr);

		std::vector<Udata> send_all(user& sender, user& target, std::string msg, int remocon);

		std::vector<user>& get_users();
		bool operator==(const Chan& t) const;
		void seterror();
		int geterror();
};
/*
///@ brief 기본형 : 나 빼고 다 보냄, flag 1 -> PART
*/
std::vector<Udata> Chan::send_all(user& sender, user& target, std::string msg, int remocon)
{
	std::vector<Udata>			ret;
	std::vector<user>::iterator it;

	if (is_user(sender) == false)
	{
		//throw noUserException();
	}

	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		Udata packet;

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
				packet.msg = msg;
				break ;
			case TOPIC:
				packet.msg = msg;
				break ;
		}
		ret.push_back(packet);
	}
	return ret;
}

std::string Chan::get_name()
{
	return this->name_;
}

/// @brief 있으면1, 없으면 0
bool Chan::is_user(user& usr)
{
	std::vector<user>::iterator it;

	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->nickname_ == usr.nickname_)
			return 1;
	}
	return 0;
}

void Chan::add_user(user& joiner)
{
	connectors_.push_back(joiner);
}
void Chan::set_host()
{
	this->host_ = connectors_[0];
}
void Chan::set_channel_name(std::string& chan_name)
{
	this->name_ = chan_name;
}

void Chan::set_topic(std::string& topic)
{
	this->topic_ = topic;
}

std::vector<user>& Chan::get_users()
{
	return this->connectors_;
}
void	Chan::delete_user(user& usr)
{
	std::vector<user>::iterator it = std::find(this->connectors_.begin(), \
	this->connectors_.end(), usr);
	std::size_t size = std::distance(this->connectors_.begin(), it);
	this->connectors_.erase(this->connectors_.begin() + size);
}
bool Chan::operator==(const Chan& t) const
{
	return (this->name_ == t.name_);
}
user Chan::get_host()
{
	return this->host_;
}

std::string Chan::get_info()
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
