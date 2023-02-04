#include "Channels.hpp"

bool	Channels::is_channel(std::string& chan_name)
{
	std::vector<ChatRoom>::iterator it;

	for (it = Channels_.begin(); it != Channels_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return true;
		}
	}
	return false;
}

bool	Channels::is_user_in_channel(ChatUser& leaver)
{
	std::vector<ChatRoom>::iterator it = Channels_.begin();
	for (; it != Channels_.end(); ++it)
	{
		if (it->is_user(leaver))
		{
			return true;
		}
	}
	return false;	
}

void	Channels::create_channel(ChatUser& joiner, std::string& chan_name, std::string chan_access)
{
	ChatRoom	tmp;

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
	ChatRoom tmp;

	tmp.set_channel_name(chan_name);

	std::vector<ChatRoom>::iterator it = std::find(Channels_.begin(), \
	Channels_.end(), tmp);
	std::size_t size = std::distance(this->Channels_.begin(), it);
	this->Channels_.erase(this->Channels_.begin() + size);
}

/// @brief sender, command -> error_message에 보낼 정보
ChatRoom&	Channels::select_channel(std::string& chan_name, int error_code, ChatUser& sender, std::string& command) // 403 ERROR Sender::no_channel_message
{
	std::vector<ChatRoom>::iterator it = Channels_.begin();
	for (; it != Channels_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return *it;
		}
	}
	//Sender::error_message(sender.client_sock_, command, error_code);
	return *it;
}

ChatRoom&	Channels::select_channel(ChatUser& connector, int error_code, std::string& command) // 476 ERROR Sender::join_invaild_channel_name_message
{
	std::vector<ChatRoom>::iterator it = Channels_.begin();
	for (; it != Channels_.end(); ++it)
	{
		if (it->is_user(connector))
		{
			return *it;
		}
	}
	//Sender::error_message(connector.client_sock_, command, error_code);
	return *it;
}


Udata	Channels::join_channel(ChatUser& joiner, std::string& chan_name)
{
	Udata				ret;
	Event				tmp;
	std::string			command("JOIN");

	ChatRoom& chan = select_channel(chan_name, 476, joiner, command);
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

	return ret;
}

Udata	Channels::leave_channel(ChatUser&leaver, std::string& chan_name, std::string& msg)
{
	Event				tmp;
	Udata				res;
	std::string			command("PART");

	ChatRoom& chan = select_channel(chan_name, 403, leaver, command); // 403 ERROR Sender::no_channel_message
	//유저가 존재하지 않을 경우(로비에서 part하면 예외처리)
	if (chan.is_user(leaver) == 0)
	{
		return res;
	}
	//Msg전송 : PART 내용에 따라 전송 -> 아마 채널의 다른 유저들에게 떠났다고 알려줘야
	std::vector<ChatUser> users = chan.get_users();
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
Udata	Channels::channel_msg(ChatUser& sender, std::string chan_name, std::string& msg)
{
	Udata		ret;
	Event		tmp;
	std::string	command("PRIVMSG");

	ChatRoom&	channel = select_channel(chan_name, 403, sender, command); // 403 ERROR Sender::no_channel_message
	ret = channel.send_all(sender, sender, msg, PRIV);
	return ret;
}

Udata	Channels::channel_notice(ChatUser& sender, std::string chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;
	std::string		command("NOTICE");

	ChatRoom&	channel = select_channel(chan_name, 403, sender, command); // 403 ERROR Sender::no_channel_message
	ret = channel.send_all(sender, sender, msg, NOTICE);
	return ret;
}

Udata	Channels::channel_wall(ChatUser& sender, std::string chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;
	std::string		command("WALL");

	ChatRoom&	channel = select_channel(chan_name, 403, sender, command); // 403 ERROR Sender::no_channel_message
	ChatUser	host = channel.get_host();

	if (host == sender)
	{
		std::cout << "[Debug] " << "host == sender" << std::endl;
		return ret;
	}
	tmp = Sender::wall_message(sender, channel.get_host(), chan_name, msg);
	ret.insert(tmp);
	return ret;
}

Udata	Channels::kick_channel(ChatUser& host, ChatUser& target, std::string& chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;
	std::string		command("PRIVMSG");


	ChatRoom&	channel = select_channel(chan_name, 403, host, command); // 403 ERROR Sender::no_channel_message
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

Udata	Channels::who_channel(ChatUser& asker, std::string& chan_name)
{
	Udata	ret;
	Event	tmp;
	std::string command("WHO");

	ChatRoom& chan = select_channel(chan_name, 315, asker, command); // 315 -> end of list만 보낸다.

	tmp = Sender::who_joiner_352_message(asker, chan_name);
	tmp.second += Sender::who_315_message(asker, chan_name);

	ret.insert(tmp);
	return ret;
}

Udata	Channels::quit_channel(ChatUser& leaver, std::string msg)
{
	Udata			ret;
	Event			tmp;
	std::string		command("QUIT");

	ChatRoom& channel = select_channel(leaver, 401, command); // 401 no_such_nick
	ret = channel.send_all(leaver, leaver, msg, QUIT);
	channel.delete_user(leaver);
	
	return ret;
}

Udata	Channels::mode_channel(ChatUser& moder, const std::string& chan_name, const bool vaild)
{
	Udata	ret;
	Event	tmp;

	std::time_t result = std::time(NULL);
	std::stringstream	ss;
	ss << result;
	const std::string& time_stamp(ss.str());

	if (vaild)	// true
	{
		tmp = Sender::mode_324_message(moder, chan_name);
		tmp.second += Sender::mode_329_message(moder, chan_name, time_stamp);
		ret.insert(tmp);
	}

	return ret;
}

Udata	Channels::nick_channel(ChatUser& nicker, std::string& send_msg)
{
	Udata		ret;
	ChatUser	trash;
	std::string command("NICK");
	
	ChatRoom& channel = select_channel(nicker, 401, command); // 401 no such nick

	channel.change_nick(nicker, send_msg);
	ret = channel.send_all(nicker, trash, send_msg, NICK);
	return ret;
}

Udata	Channels::set_topic(ChatUser& sender, std::string& chan_name, std::string& topic)
{
	Udata			ret;
	Event			tmp;
	std::string		command("TOPIC");


	ChatRoom& channel = select_channel(chan_name, 403, sender, command); // 403 ERROR Sender::no_channel_message
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
	return	ret;
}
