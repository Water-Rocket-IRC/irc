#pragma once

#include "Chan.hpp"

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
		void 				create_channel(user& joiner, std::string& chan_name);
		void 				delete_channel(std::string& chan_name);
		Chan&				select_channel(std::string& chan_name);
		Chan&				select_channel(user& connector);

		std::vector<Udata>	set_topic(user& sender, std::string& chan_name, std::string& topic);
		std::vector<Udata>	kick_channel(user& host, user& target, std::string& chan_name, std::string& msg);
		std::vector<Udata>	quit_channel(user& target, std::string msg);
		std::vector<Udata> 	join_channel(user& joiner, std::string& chan_name);
		std::vector<Udata>	leave_channel(user&leaver, std::string& chan_name, std::string& msg);
		std::vector<Chan>&	get_channels() { return	Channels_; };

		class user_no_any_channel_exception : public std::exception
		{
			public:
				const char*		what(void) const throw();
		};
};

const	char*		Channels::user_no_any_channel_exception::what(void) const throw()
{
	return "err: user no any channel";
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

void	Channels::create_channel(user& joiner, std::string& chan_name)
{
	Chan	tmp;
	
	tmp.set_channel_name(chan_name);
	tmp.add_user(joiner);
	tmp.set_host();
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
	//result != Channels_.end() ? true : false; //throw NoMatchChannelsException();
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
	throw	user_no_any_channel_exception();
	return *it;
}

std::vector<Udata>	Channels::join_channel(user& joiner, std::string& chan_name)
{
	Udata				tmp;
	std::vector<Udata>	res;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::join_message(joiner, joiner, chan_name);//이거 수정해야함
		res.push_back(tmp);
		this->create_channel(joiner, chan_name);
	}
	else
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
			res = chan.send_all(joiner, "Join \"" + chan_name + "\" channel, " + joiner.nickname_, JOIN);

		}
		std::vector<user>::iterator it;
		for (it = chan.get_users().begin(); it != chan.get_users().end(); it++)
		{
			std::cout << it->nickname_ << std::endl;
		}
	}
	//유저가 채널에 성공적으로 입장했다는 메시지 전송 + 서버 정보 전송
	return res;
}

std::vector<Udata>	Channels::leave_channel(user&leaver, std::string& chan_name, std::string& msg)
{
	Udata				tmp;
	std::vector<Udata>	res;
	
	if (is_channel(chan_name) == false)
	{
		tmp.msg = "There is no channel" + leaver.nickname_;
		res.push_back(tmp);
		//채널이 없을 경우, 오류 메시지 유저에게 전송, 이건 클라이언트를 통해 들어올 수 있음
	}
	else
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
		res = chan.send_all(leaver, msg, PART);
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
	return res;
}

/// @brief 채널 전체 유저에게 메시지 전달. 내외부 모두 사용됨
std::vector<Udata>	Channels::channel_msg(user& sender, std::string chan_name, std::string& msg)
{
	std::vector<Udata>	ret;
	Udata				tmp;

	if (is_channel(chan_name) == false)
	{
		tmp.msg = "No such Channel";
		ret.push_back(tmp);
		//채널이 없을 경우, 오류 메시지 유저에게 전송
		return ret;
	}
	try
	{
		Chan&	channel = select_channel(chan_name);
		ret = channel.send_all(sender, msg, PRIV);
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << 
	}
	return ret;
}

std::vector<Udata>	Channels::channel_notice(user& sender, std::string chan_name, std::string& msg)
{
	std::vector<Udata>	ret;
	Udata				tmp;

	if (is_channel(chan_name) == false)
	{
		tmp.msg = "No such Channel";
		ret.push_back(tmp);
		//채널이 없을 경우, 오류 메시지 유저에게 전송
		return ret;
	}
	Chan&	channel = select_channel(chan_name);

	//본인에겐 빼고 보내야함
	ret = channel.send_all(sender, msg, PRIV);
	return ret;
}

Udata	Channels::channel_wall(user& sender, std::string chan_name, std::string& msg)
{
	Udata				ret;

	if (is_channel(chan_name) == false)
	{
		ret.msg = "No such Channel";
		//채널이 없을 경우, 오류 메시지 유저에게 전송
	}
	Chan&	channel = select_channel(chan_name);
	user				host;
	host = channel.get_host();

	if (host == sender)
	{
		std::cout << "[Debug] " << "host == sender" << std::endl;
		return ret;
	}
	ret.msg = sender.nickname_ + " need to " + chan_name + "\'s host, " + host.nickname_;
	return ret;
}

std::vector<Udata>	Channels::kick_channel(user& host, user& target, std::string& chan_name, std::string& msg)
{
	std::vector<Udata>	ret;
	Udata 				tmp;

	if (is_channel(chan_name) == false)
	{
		//
		ret.push_back(tmp);
	}
	Chan& channel = select_channel(chan_name);
	if (channel.get_host() == host)
	{
		if (channel.is_user(target) == true)
		{
			ret = channel.send_all(host, msg, KICK); //add target
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
		ret = channel.send_all(target, msg, QUIT);
		channel.delete_user(target);
	}
	catch (std::exception& e)
	{
		tmp.msg = e.what();
		ret.push_back(tmp);
	}
	return ret;
}

std::vector<Udata>	Channels::set_topic(user& sender, std::string& chan_name, std::string& topic)
{
	std::vector<Udata> ret;

	try
	{
		Chan& channel = select_channel(chan_name);
		if (channel.get_host() == sender)
		{
			std::string topic_msg = "Topic was changed to " + topic;
			channel.set_topic(topic);
			ret = channel.send_all(sender,topic_msg, TOPIC);
		}
		else
		{
			std::string topic_msg = "You do not have access to change the topic on this channel";
			ret = channel.send_all(sender, topic_msg, TOPIC);
		}
	}
	catch (std::exception& e)
	{
		Udata 	tmp;
		tmp.msg = e.what();
		ret.push_back(tmp);
	}
	return	ret;
}
