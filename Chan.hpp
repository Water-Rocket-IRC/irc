#pragma once

#include "Sender.hpp"
#include "Udata.hpp"
#include <vector>
#include <algorithm>
#include <sstream>
#include "user.hpp"


enum e_send_switch { JOIN, PART, PRIV, KICK, QUIT, NOTICE, TOPIC, WALL, NICK, MODE, WHO};
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
		std::string			access_;


//TODO: 메시지를 전송하는 모든 명령은 udata로 리턴할 것. Udata.hpp 참고
	public:
		std::string			get_info(void);

		std::string&		get_name(void);
		user&				get_host(void);
		void				set_host(void);
		std::string&		get_access(void);
		void				set_access(const std::string& access);
		void				set_topic(std::string& topic);
		bool 				is_user(user& usr);
		void				change_nick(user& usr, std::string new_nick);
		void 				add_user(user& joiner);
		void 				set_channel_name(std::string& chan_name);
		void				delete_user(user& usr);
		std::string			get_user_list_str(void);


		//std::vector<Udata>	send_all(user& sender, user& target, std::string msg, int remocon);
		Udata	send_all(user& sender, user& target, std::string msg, int remocon);

		std::vector<user>&	get_users(void);
		std::vector<user>	sort_users(void);
		bool operator==(const Chan& t) const;
		void seterror();
		int geterror();
};

// void	DebugshowUsers(std::vector<user>& target) {
// 	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
// 	for (std::vector<user>::iterator it = target.begin(); it != target.end(); ++it) {
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

std::string&	Chan::get_access(void) { return access_; }
void			Chan::set_access(const std::string& access) { access_ = access; }

void			Chan::change_nick(user& usr, std::string new_nick)
{
	std::vector<user>::iterator it;
	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->nickname_ == usr.nickname_)
		{
			usr.nickname_ = new_nick;
			break ;
		}
	}
}

std::vector<user>	Chan::sort_users(void)
{
	std::vector<user>	ret;

	ret.push_back(connectors_.at(0));
	if (connectors_.size() == 1)
		return ret;
	for (std::vector<user>::iterator it = connectors_.begin()
		 ; it != connectors_.end(); ++it)
	{	
		std::vector<user>::iterator ret_it = ret.begin();
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

std::string		Chan::get_user_list_str(void)
{
	std::string					ret;
	std::vector<user> sort = sort_users();
	for (std::vector<user>::iterator it = sort.begin(); it != sort.end(); ++it)
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
Udata Chan::send_all(user& sender, user& target, std::string msg, int remocon)
{
	Udata			ret;
	std::vector<user>::iterator it;

	if (is_user(sender) == false)
	{
		//Sender::error_message(sender.client_sock_, "")
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

std::string&	Chan::get_name(void)
{
	return this->name_;
}

/// @brief 있으면1, 없으면 0
bool Chan::is_user(user& usr)
{
	std::vector<user>::iterator it;

	for (it = connectors_.begin(); it != connectors_.end(); it++)
	{
		if (it->client_sock_ == usr.client_sock_)
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
user& Chan::get_host()
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
