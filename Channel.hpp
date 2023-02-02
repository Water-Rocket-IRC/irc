#pragma once

#include "Chan.hpp"
#include "Users.hpp"

class Channels
{
	private:
		std::vector<Chan> Channels_;

	public:
	//메시지를 전송하는 모든 명령은 std::vector<Udata>로 리턴할 것
		std::vector<Udata>	channel_msg(user& sender, std::string chan_name, std::string& msg);
		std::vector<Udata>	channel_notice(user& sender, std::string chan_name, std::string& msg);
		Udata				channel_wall(user& sender, std::string chan_name, std::string& msg);

		bool				is_channel(std::string& chan_name);
		void 				create_channel(user& joiner, std::string& chan_name, std::string chan_access);
		void 				delete_channel(std::string& chan_name);
		Chan&				select_channel(std::string& chan_name);
		Chan&				select_channel(user& connector);

		std::vector<Udata>	set_topic(user& sender, std::string& chan_name, std::string& topic);
		std::vector<Udata>	kick_channel(user& host, user& target, std::string& chan_name, std::string& msg);
		std::vector<Udata>	quit_channel(user& target, std::string msg);
		std::vector<Udata> 	join_channel(user& joiner, std::string& chan_name);
		std::vector<Udata>	leave_channel(user&leaver, std::string& chan_name, std::string& msg);
		std::vector<Udata>	nick_channel(user& who, std::string& send_msg);
		std::vector<Chan>&	get_channels() { return	Channels_; };

		class user_no_any_channels_exception : public std::exception
		{
			public:
				const char*		what(void) const throw();
		};
		class no_match_channels_exception : public std::exception
		{
			public:
				const char*		what(void) const throw();
		};
};

static void	DebugshowUsers(std::vector<user>& target) {
	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
	for (std::vector<user>::iterator it = target.begin(); it != target.end(); ++it) {
		std::cout << (*it).nickname_ << " ➜ ";
	}
	std::cout << YELLOW << "(end)" << RESET << std::endl;
}
static void	DebugshowChannels(std::vector<Chan>& target) {
	std::cout << YELLOW << "(begin) " << RESET << " ➜ ";
	for (std::vector<Chan>::iterator it = target.begin(); it != target.end(); ++it) {
		std::cout << (*it).get_name() << " ➜ ";
	}
	std::cout << YELLOW << "(end)" << RESET << std::endl;
}

const	char*		Channels::user_no_any_channels_exception::what(void) const throw()
{
	return "err: user no any channel";
}

const	char*		Channels::no_match_channels_exception::what(void) const throw()
{
	return "err: no match channel";
}

bool	Channels::is_channel(std::string& chan_name)
{
	std::vector<Chan>::iterator it;

	for (it = Channels_.begin(); it != Channels_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return true;
		}
	}
	return false;
}

void	Channels::create_channel(user& joiner, std::string& chan_name, std::string chan_access)
{
	Chan	tmp;

	tmp.set_channel_name(chan_name);
	tmp.add_user(joiner);
	tmp.set_host();
	tmp.set_access(chan_access);
	Channels_.push_back(tmp);
}
 /*
 * operator 가 없을 때 (0 index가 비어있다면 삭제)
 */
void	Channels::delete_channel(std::string& chan_name)
{
	Chan tmp;

	tmp.set_channel_name(chan_name);

	std::vector<Chan>::iterator it = std::find(Channels_.begin(), \
	Channels_.end(), tmp);
	std::size_t size = std::distance(this->Channels_.begin(), it);
	this->Channels_.erase(this->Channels_.begin() + size);
}

Chan&	Channels::select_channel(std::string& chan_name)
{
	Chan tmp;

	tmp.set_channel_name(chan_name);
	std::vector<Chan>::iterator result = std::find(Channels_.begin(), \
	Channels_.end(), tmp);
	result != Channels_.end() ? true : throw no_match_channels_exception();
	return *result;
}

Chan&	Channels::select_channel(user& connector)
{
	std::vector<Chan>::iterator it = Channels_.begin();
	for (; it != Channels_.end(); ++it)
	{
		if (it->is_user(connector))
		{
			return *it;
		}
	}
	throw	user_no_any_channels_exception();
	return *it;
}

std::vector<Udata>	Channels::join_channel(user& joiner, std::string& chan_name)
{
	Udata				tmp;
	std::vector<Udata>	res;

	try
	{
		Chan& chan = select_channel(chan_name);

		if (chan.is_user(joiner) == true)
		{
			tmp.msg = "Already in channel, " + joiner.nickname_;
			res.push_back(tmp);
		}
		else
		{
			chan.add_user(joiner);
			res = chan.send_all(joiner, joiner, "Join \"" + chan_name + "\" channel, " + joiner.nickname_, JOIN);
		}
	}
	catch(const std::exception&)
	{
		// tmp = Sender::join_message(joiner, joiner, chan_name);	//이거 수정해야함
		res.push_back(Sender::join_message(joiner, joiner, chan_name));
		this->create_channel(joiner, chan_name, "=");
	}

	// 추가하기
	Chan& chan = select_channel(chan_name);
	const std::string& chan_user_list = chan.get_user_list_str();
	res.at(0).msg += Sender::join_353_message(joiner, chan.get_name(), chan.get_access(), chan_user_list);
	res.at(0).msg += Sender::join_366_message(joiner, chan.get_name());

	//유저가 채널에 성공적으로 입장했다는 메시지 전송 + 서버 정보 전송
	return res;
}

std::vector<Udata>	Channels::leave_channel(user&leaver, std::string& chan_name, std::string& msg)
{
	Udata				tmp;
	std::vector<Udata>	res;

	try
	{
		Chan& chan = select_channel(chan_name);
		//유저가 존재하지 않을 경우(로비에서 part하면 예외처리)
		if (chan.is_user(leaver) == 0)
		{
			return res;
		}
		//Msg전송 : PART 내용에 따라 전송 -> 아마 채널의 다른 유저들에게 떠났다고 알려줘야
		std::vector<user> users = chan.get_users();
		int user_size = users.size();
		//PART하면, 그 내역은 모두에게 보내진다. 나간 사람 포함한다.
		res = chan.send_all(leaver, leaver, msg, PART);
		chan.delete_user(leaver);

		std::vector<user>::iterator it;
		for (it = chan.get_users().begin(); it != chan.get_users().end(); it++)
		{
			std::cout << it->nickname_ << std::endl;
		}

		if (user_size == 1) //채널 잘 삭제되는 것 확인
		{
			delete_channel(chan_name);
		}
		else
		{
			if (leaver == chan.get_host())
			{
				chan.set_host();
			}
		}

		std::vector<Chan>::iterator ite;
		for (ite = Channels_.begin(); ite != Channels_.end(); ite++)
		{
			std::cout << ite->get_name() << std::endl;
		}
	}
	catch (std::exception&)
	{
		// sender
		// tmp = Sender::
		tmp.msg = "There is no channel " + leaver.nickname_;
		res.push_back(tmp);
	}

	return res;
}

/// @brief 채널 전체 유저에게 메시지 전달. 내외부 모두 사용됨
std::vector<Udata>	Channels::channel_msg(user& sender, std::string chan_name, std::string& msg)
{
	std::vector<Udata>	ret;
	Udata				tmp;

	try
	{
		Chan&	channel = select_channel(chan_name);
		ret = channel.send_all(sender, sender, msg, PRIV);
	}
	catch (std::exception&)
	{
		tmp.msg = "No such Channel";
		ret.push_back(tmp);
	}
	return ret;
}

std::vector<Udata>	Channels::channel_notice(user& sender, std::string chan_name, std::string& msg)
{
	std::vector<Udata>	ret;
	Udata				tmp;

	try
	{
		Chan &channel = select_channel(chan_name);
		ret = channel.send_all(sender, sender, msg, PRIV);
	}
	catch(const std::exception&)
	{
		tmp.msg = "No such Channel";
		ret.push_back(tmp);
	}
	return ret;
}

Udata	Channels::channel_wall(user& sender, std::string chan_name, std::string& msg)
{
	Udata				ret;

	try
	{
		Chan&	channel = select_channel(chan_name);
		user				host;
		host = channel.get_host();

		if (host == sender)
		{
			std::cout << "[Debug] " << "host == sender" << std::endl;
			return ret;
		}
		ret.msg = sender.nickname_ + " need to " + chan_name + "\'s host, " + host.nickname_;
	}
	catch (std::exception&)
	{
		ret = Sender::no_channel_message(sender, chan_name);
	}
	return ret;
}

std::vector<Udata>	Channels::kick_channel(user& host, user& target, std::string& chan_name, std::string& msg)
{
	std::vector<Udata>	ret;
	Udata 				tmp;

	try
	{
		Chan& channel = select_channel(chan_name);
		if (channel.get_host() == host)
		{
			if (channel.is_user(target) == true)
			{
				ret = channel.send_all(host, target, msg, KICK); //add target
				channel.delete_user(target);
			}
			else
			{
				tmp = Sender::kick_error_no_user_message(host, host.nickname_, target.nickname_, chan_name);
				ret.push_back(tmp);
			}
		}
		else
		{
			tmp = Sender::kick_error_not_op_message(host, channel.get_host().nickname_, chan_name);
			ret.push_back(tmp);
		}
	}
	catch (std::exception&)
	{
		tmp = Sender::no_channel_message(host, chan_name);
		ret.push_back(tmp);
	}
	return ret;
}

std::vector<Udata>	Channels::quit_channel(user& target, std::string msg)
{
	std::vector<Udata>	ret;
	Udata 				tmp;

	try
	{
		//어떤 채널도 없으면 catch로 감
		Chan& channel = select_channel(target);
		ret = channel.send_all(target, target, msg, QUIT);
		channel.delete_user(target);
	}
	catch (const std::exception& e)
	{
		std::string		chan_name = "#";
		tmp = Sender::no_channel_message(target, chan_name);					//No such user in every channel
		ret.push_back(tmp);
	}
	return ret;
}

std::vector<Udata>	Channels::nick_channel(user& nicker, std::string& send_msg)
{
	std::vector<Udata> ret;

	user	trash;

	Chan& channel = select_channel(nicker);
	ret = channel.send_all(nicker, trash, send_msg, NICK);
	return ret;
}

std::vector<Udata>	Channels::set_topic(user& sender, std::string& chan_name, std::string& topic)
{
	Udata 				tmp;
	std::vector<Udata>	ret;

	try
	{
		Chan& channel = select_channel(chan_name);
		if (channel.get_host() == sender)
		{
			std::string topic_msg = "Topic was changed to " + topic;
			channel.set_topic(topic);
			ret = channel.send_all(sender, sender, topic_msg, TOPIC);
		}
		else
		{
			std::string topic_msg = "You do not have access to change the topic on this channel";
			ret = channel.send_all(sender, sender, topic_msg, TOPIC);
		}
	}
	catch (std::exception&)
	{
		ret.push_back(Sender::no_channel_message(sender, chan_name));
	}
	return	ret;
}
