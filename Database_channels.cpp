#include "Channel.hpp"
#include "Database.hpp"
#include "Udata.hpp"
#include "debug.hpp"

bool	Database::is_channel(std::string& chan_name)
{
	std::vector<Channel>::iterator it;

	for (it = channel_list_.begin(); it != channel_list_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return true;
		}
	}
	return false;
}

bool	Database::is_user_in_channel(User& leaver)
{
	std::vector<Channel>::iterator it = channel_list_.begin();
	for (; it != channel_list_.end(); ++it)
	{
		if (it->is_user(leaver))
		{
			return true;
		}
	}
	return false;	
}

void	Database::create_channel(User& joiner, std::string& chan_name, std::string chan_access)
{
	Channel	tmp;

	tmp.set_channel_name(chan_name);
	tmp.add_user(joiner);
	tmp.set_host();
	tmp.set_access(chan_access);
	channel_list_.push_back(tmp);
}
 /*
 * operator 가 없을 때 (0 index가 비어있다면 삭제)
 */
void	Database::delete_channel(std::string& chan_name)
{
	Channel tmp;

	tmp.set_channel_name(chan_name);

	std::vector<Channel>::iterator it = std::find(channel_list_.begin(), \
	channel_list_.end(), tmp);
	std::size_t size = std::distance(this->channel_list_.begin(), it);
	this->channel_list_.erase(this->channel_list_.begin() + size);
}

/// @brief sender, command -> error_message에 보낼 정보
Channel&	Database::select_channel(std::string& chan_name) // 403 ERROR Sender::no_channel_message
{
	std::vector<Channel>::iterator it = channel_list_.begin();
	for (; it != channel_list_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return *it;
		}
	}
	return *it;
}

Channel&	Database::select_channel(User& connector) // 476 ERROR Sender::join_invaild_channel_name_message
{
	std::vector<Channel>::iterator it = channel_list_.begin();
	for (; it != channel_list_.end(); ++it)
	{
		if (it->is_user(connector))
		{
			return *it;
		}
	}
	return *it;
}


Udata	Database::join_channel(User& joiner, const std::string& chan_name_)
{
	Udata		ret;
	Event		tmp;
	std::string	chan_name(chan_name_);

	if (is_channel(chan_name) == false)
	{
		create_channel(joiner, chan_name, "="); // "="는 public으로 만드는 것
		tmp = Sender::join_message(joiner, joiner, chan_name);
		ret.insert(tmp);
		Channel& chan = select_channel(chan_name);
		Udata_iter it = ret.find(joiner.client_sock_);
		it->second += Sender::join_353_message(joiner, chan.get_name(), chan.get_access(), "@" + joiner.nickname_);
		it->second += Sender::join_366_message(joiner, chan.get_name());
	}
	else
	{
		Channel& chan = select_channel(chan_name);
		chan.add_user(joiner);
		const std::string& chan_user_list(chan.get_user_list_str());
		ret = chan.send_all(joiner, joiner, "Join \"" + chan_name + "\" channel, " + joiner.nickname_, JOIN);
		Udata_iter it = ret.find(joiner.client_sock_);
		it->second += Sender::join_353_message(joiner, chan.get_name(), chan.get_access(), chan_user_list);
		it->second += Sender::join_366_message(joiner, chan.get_name());
	}
	// chan.add_user(joiner);
	debug::showChannels(channel_list_);
	return ret;
}

Udata	Database::leave_channel(User&leaver, std::string& chan_name, const std::string& msg_)
{
	Event				tmp;
	Udata				ret;
	std::string			msg(msg_);

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(leaver, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel& chan = select_channel(chan_name); // 403 ERROR Sender::no_channel_message
	//유저가 존재하지 않을 경우(로비에서 part하면 예외처리)
	if (chan.is_user(leaver) == 0)
	{
		tmp = Sender::no_user_message(leaver, leaver.nickname_);
		return ret;
	}
	//Msg전송 : PART 내용에 따라 전송 -> 아마 채널의 다른 유저들에게 떠났다고 알려줘야
	std::vector<User> users = chan.get_users();
	int user_size = users.size();
	//PART하면, 그 내역은 모두에게 보내진다. 나간 사람 포함한다.
	ret = chan.send_all(leaver, leaver, msg, QUIT);
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
	return ret;
}

/// @brief 채널 전체 유저에게 메시지 전달. 내외부 모두 사용됨
Udata	Database::channel_msg(User& sender, std::string chan_name, std::string& msg)
{
	Udata		ret;
	Event		tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel&	channel = select_channel(chan_name); // 403 ERROR Sender::no_channel_message
	ret = channel.send_all(sender, sender, msg, PRIV);
	return ret;
}

Udata	Database::channel_notice(User& sender, std::string chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel&	channel = select_channel(chan_name); // 403 ERROR Sender::no_channel_message
	ret = channel.send_all(sender, sender, msg, NOTICE);
	return ret;
}

Udata	Database::channel_wall(User& sender, std::string chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel&	channel = select_channel(chan_name); // 403 ERROR Sender::no_channel_message
	User	host = channel.get_host();
	if (host == sender)
	{
		std::cout << "[Debug] " << "host == sender" << std::endl;
		return ret;
	}
	tmp = Sender::wall_message(sender, channel.get_host(), chan_name, msg);
	ret.insert(tmp);
	return ret;
}

Udata	Database::kick_channel(User& host, User& target, std::string& chan_name, std::string& msg)
{
	Udata			ret;
	Event			tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(host, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel&	channel = select_channel(chan_name); // 403 ERROR Sender::no_channel_message
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

Udata	Database::who_channel(User& asker, std::string& chan_name)
{
	Udata	ret;
	Event	tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(asker, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel& chan = select_channel(chan_name); // 315 -> end of list만 보낸다.
	tmp = Sender::who_joiner_352_message(asker, chan_name);
	tmp.second += Sender::who_315_message(asker, chan_name);

	ret.insert(tmp);
	return ret;
}

Udata	Database::quit_channel(User& leaver, std::string msg)
{
	Udata			ret;
	Event			tmp;

	if (is_user_in_channel(leaver) == false)
	{
		tmp = Sender::no_user_message(leaver, leaver.nickname_);
		ret.insert(tmp);
		return ret;
	}
	Channel& channel = select_channel(leaver); // 401 no_such_nick
	ret = channel.send_all(leaver, leaver, msg, QUIT);
	channel.delete_user(leaver);
	
	return ret;
}

Udata	Database::mode_channel(User& moder, std::string& chan_name, bool vaild)
{
	Udata	ret;
	Event	tmp;

	std::time_t result = std::time(NULL);
	std::stringstream	ss;
	ss << result;
	const std::string& time_stamp(ss.str());

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(moder, chan_name);
		ret.insert(tmp);
		return ret;
	}

	if (vaild)	// true
	{
		tmp = Sender::mode_324_message(moder, chan_name);
		tmp.second += Sender::mode_329_message(moder, chan_name, time_stamp);
		ret.insert(tmp);
	}

	return ret;
}

Udata	Database::nick_channel(User& nicker, std::string& send_msg)
{
	Udata		ret;
	Event		tmp;
	User		trash;

	Channel& channel = select_channel(nicker); // 401 no such nick

	channel.change_nick(nicker, send_msg);
	ret = channel.send_all(nicker, trash, send_msg, NICK);

	return ret;
}

Udata	Database::set_topic(User& sender, std::string& chan_name, std::string& topic)
{
	Udata			ret;
	Event			tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel& channel = select_channel(chan_name); // 403 ERROR Sender::no_channel_message
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
