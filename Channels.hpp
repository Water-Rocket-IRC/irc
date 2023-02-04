#ifndef CHANNELS_HPP
# define CHANNELS_HPP

#include "Chan.hpp"
#include "Udata.hpp"
#include "Users.hpp"

/*********************** TODO: 왜 채널의 기본 생성자가 없다고 뜨죠...?? *********************/
class Channels
{
	private:
		std::vector<Chan> Channels_;

	public:
	//TODO: 메시지를 전송하는 모든 명령은 udata로 리턴할 것. map안에 집어넣으면 됨. Udata hpp참고
		Udata				channel_msg(user& sender, std::string chan_name, std::string& msg);
		Udata				channel_notice(user& sender, std::string chan_name, std::string& msg);
		Udata				channel_wall(user& sender, std::string chan_name, std::string& msg);

		bool				is_channel(std::string& chan_name);
		bool				is_user_in_channel(user& leaver);
		void 				create_channel(user& joiner, std::string& chan_name, std::string chan_access);
		void 				delete_channel(std::string& chan_name);
		Chan&				select_channel(std::string& chan_name, int error_code);
		Chan&				select_channel(user& connector, int error_code);

		Udata				set_topic(user& sender, std::string& chan_name, std::string& topic);
		Udata				kick_channel(user& host, user& target, std::string& chan_name, std::string& msg);
		Udata				quit_channel(user& target, std::string msg);
		Udata			 	join_channel(user& joiner, std::string& chan_name);
		Udata				leave_channel(user&leaver, std::string& chan_name, std::string& msg);
		Udata				nick_channel(user& who, std::string& send_msg);
		Udata				mode_channel(user& moder, const std::string& chan_name, const bool vaild);
		Udata				who_channel(const uintptr_t& sock, std::string& chan_name);
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

Udata	Channels::who_channel(const uintptr_t& sock, std::string& chan_name)
{
	Udata	ret;	
	Chan& chan = select_channel(chan_name, 0); // end of list만 보낸다. error 없음
	
	ret.insert((tmp));
	return ret;
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

bool	Channels::is_user_in_channel(user& leaver)
{
	std::vector<Chan>::iterator it = Channels_.begin();
	for (; it != Channels_.end(); ++it)
	{
		if (it->is_user(leaver))
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

Chan&	Channels::select_channel(std::string& chan_name, int error_code) // 403 ERROR Sender::no_channel_message
{
	std::vector<Chan>::iterator it = Channels_.begin();
	for (; it != Channels_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return *it;
		}
	}
	Sender::error_message(error_code); //근데 이거 어딨지?
	return *it;
}

Chan&	Channels::select_channel(user& connector, int error_code) // 476 ERROR Sender::join_invaild_channel_name_message
{
	std::vector<Chan>::iterator it = Channels_.begin();
	for (; it != Channels_.end(); ++it)
	{
		if (it->is_user(connector))
		{
			return *it;
		}
	}
	throw user_no_any_channels_exception();
	return *it;
}


Udata	Channels::join_channel(user& joiner, std::string& chan_name)
{
	Udata				ret;
	Event				tmp;

	try
	{
		Chan& chan = select_channel(chan_name, 476);
		if (chan.is_user(joiner) == false)
		{
			tmp = Sender::no_user_message(joiner, chan_name);
			ret.insert(tmp);
		}
		else
		{
			chan.add_user(joiner);
			ret = chan.send_all(joiner, joiner, "Join \"" + chan_name + "\" channel, " + joiner.nickname_, JOIN);
			const std::string& chan_user_list = chan.get_user_list_str();

			Udata_iter it = ret.find(joiner.client_sock_);
			it->second += Sender::join_353_message(joiner, chan.get_name(), chan.get_access(), chan_user_list);
			it->second += Sender::join_366_message(joiner, chan.get_name());
		}
	}
	catch(const std::exception& e) //여기선 채널 없으면 새로 만들어야해서 밖으로 빠지면 안됨, 아무튼 채널 새로 만듦
	{
		ret.insert(Sender::join_message(joiner, joiner, chan_name));
		create_channel(joiner, chan_name, "=");
	}
	// 추가하기



	//유저가 채널에 성공적으로 입장했다는 메시지 전송 + 서버 정보 전송
	return ret;
}

Udata	Channels::leave_channel(user&leaver, std::string& chan_name, std::string& msg)
{
	Event				tmp;
	Udata				res;

	Chan& chan = select_channel(chan_name, 403); // 403 ERROR Sender::no_channel_message
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
	return res;
}

/// @brief 채널 전체 유저에게 메시지 전달. 내외부 모두 사용됨
Udata	Channels::channel_msg(user& sender, std::string chan_name, std::string& msg)
{
	Udata		ret;
	Event		tmp;

	Chan&	channel = select_channel(chan_name, 403); // 403 ERROR Sender::no_channel_message
	ret = channel.send_all(sender, sender, msg, PRIV);
	return ret;
}

Udata	Channels::channel_notice(user& sender, std::string chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;

	Chan &channel = select_channel(chan_name, 403); // 403 ERROR Sender::no_channel_message
	ret = channel.send_all(sender, sender, msg, NOTICE);
	return ret;
}

Udata	Channels::channel_wall(user& sender, std::string chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;

	Chan&	channel = select_channel(chan_name, 403); // 403 ERROR Sender::no_channel_message
	user	host = channel.get_host();

	if (host == sender)
	{
		std::cout << "[Debug] " << "host == sender" << std::endl;
		return ret;
	}
	tmp = Sender::wall_message(sender, channel.get_host(), chan_name, msg);
	ret.insert(tmp);
	return ret;
}

Udata	Channels::kick_channel(user& host, user& target, std::string& chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;


	Chan& channel = select_channel(chan_name, 403); // 403 ERROR Sender::no_channel_message
	if (channel.get_host() == host)
	{
		if (channel.is_user(target) == true)
		{
			ret = channel.send_all(host, target, msg, KICK);
			channel.delete_user(target);
		}
		else
		{
			tmp = Sender::kick_error_no_user_message(host, host.nickname_, target.nickname_, chan_name);
			ret.insert(tmp);
		}
	}
	else
	{
		tmp = Sender::kick_error_not_op_message(host, channel.get_host().nickname_, chan_name);
		ret.insert(tmp);
	}
	
	return ret;
}

Udata	Channels::quit_channel(user& leaver, std::string msg)
{
	Udata			ret;
	Event			tmp;

	//어떤 채널도 없으면 catch로 감
	Chan& channel = select_channel(leaver, 401); // 401 no_such_nick
	ret = channel.send_all(leaver, leaver, msg, QUIT);
	channel.delete_user(leaver);
	
	return ret;
}

Udata	Channels::mode_channel(user& moder, const std::string& chan_name, const bool vaild)
{
	Udata	ret;

	user	trash;
	if (vaild)	// true
	{
		ret = Sender::mode_324_message(moder, chan_name);
		ret.msg += Sender::mode_329_message(moder, chan_name, timestamp);
	}
	else
	{
		ret =
	}
	return ret;
}

Udata	Channels::nick_channel(user& nicker, std::string& send_msg)
{
	Udata	ret;
	user	trash;
	
	Chan& channel = select_channel(nicker, 401); // 401 no such nick
	channel.change_nick(nicker, send_msg);

	ret = channel.send_all(nicker, trash, send_msg, NICK);
	return ret;
}

Udata	Channels::set_topic(user& sender, std::string& chan_name, std::string& topic)
{
	Udata			ret;
	Event			tmp;

	try
	{
		Chan& channel = select_channel(chan_name, 403); // 403 ERROR Sender::no_channel_message
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
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
	}
	return	ret;
}

#endif