#include "Channel.hpp"
#include "Database.hpp"
#include "Udata.hpp"
#include "User.hpp"
#include "color.hpp"
#include <sys/_types/_size_t.h>

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

Channel&	Database::create_channel(User& joiner, std::string& chan_name, std::string chan_access)
{
	Channel	tmp;

	tmp.set_channel_name(chan_name);
	tmp.add_user(joiner);
	tmp.set_host();
	tmp.set_access(chan_access);
	channel_list_.push_back(tmp);
	return channel_list_.back();
}

void	Database::delete_channel(std::string& chan_name)
{
	Channel tmp;

	tmp.set_channel_name(chan_name);

	std::vector<Channel>::iterator it = std::find(channel_list_.begin(), \
	channel_list_.end(), tmp);
	std::size_t size = std::distance(this->channel_list_.begin(), it);
	this->channel_list_.erase(this->channel_list_.begin() + size);
}

Channel&	Database::select_channel(std::string& chan_name)
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

Channel&	Database::select_channel(User& connector)
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

/** @brief join 요청을 받아 처리해주는 함수. 채널 생성과 입장을 처리한다 **/
Udata	Database::join_channel(User& joiner, const std::string& tmp_chan_name)
{
	Udata		ret;
	Event		tmp;
	std::string	chan_name(tmp_chan_name);


	if (is_user_in_channel(joiner))
	{
		Event	tmp2;
		tmp = Sender::join_message(joiner, joiner, chan_name);
		tmp2 = Sender::part_message(joiner, joiner, chan_name, "invaild : No Double join");
		tmp.second += tmp2.second;
		ret.insert(tmp);
		return ret;
	}
	/** 해당 이름의 채널이 없는 지 검사하기 **/
	if (is_channel(chan_name) == false)
	{
		/** 만약에 없으면 채널 만들기 **/
		Channel& chan = create_channel(joiner, chan_name, "=");
		tmp = Sender::join_message(joiner, joiner, chan_name);
		ret.insert(tmp);
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
	return ret;
}

/** @brief QUIT channel : quit할때 채널 내부 정보 정리 및 통지 **/
Udata	Database::quit_channel(User&leaver, std::string& chan_name, const std::string& msg_)
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
	Channel& chan = select_channel(chan_name);
	if (chan.is_user(leaver) == 0)
	{
		tmp = Sender::no_user_message(leaver, leaver.nickname_);
		return ret;
	}
	/** Msg전송 : PART 내용에 따라 전송 -> 채널의 다른 유저들에게 떠났다고 알려줘야 **/
	std::vector<User>& users = chan.get_users();
	const int user_size = users.size();
	ret = chan.send_all(leaver, leaver, msg, QUIT);
	chan.delete_user(leaver);

	if (user_size == 1) /** quit 상황에서 user가 1명일때 채널 삭제 **/
	{
		delete_channel(chan_name);
	}
	else
	{
		/** 호스트가 퇴장했을 경우, 권한 이양. 플라잉더치맨은 항상 선장이 필요하다 **/
		if (leaver == chan.get_host())
		{
			chan.set_host();
		}
	}
	return ret;
}

/** @brief PART 상황 처리, quit과 거의 같음 **/
Udata	Database::part_channel(User&leaver, std::string& chan_name, const std::string& msg_)
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
	Channel& chan = select_channel(chan_name);
	if (chan.is_user(leaver) == 0)
	{
		tmp = Sender::no_user_message(leaver, leaver.nickname_);
		return ret;
	}
	std::vector<User> users = chan.get_users();
	int user_size = users.size();
	ret = chan.send_all(leaver, leaver, msg, PART);
	chan.delete_user(leaver);
	if (user_size == 1)
	{
		delete_channel(chan_name);
	}
	else
	{
		if (leaver == chan.get_host())
		{
			Udata tmp_;
			chan.set_host();
		}
	}
	return ret;
}

/** @brief 채널 전체 유저에게 메시지 전달 -> join 상황에서 일반적 채팅 처리 **/
Udata	Database::channel_msg(User& sender, std::string chan_name, const std::string& msg)
{
	Udata		ret;
	Event		tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel&	channel = select_channel(chan_name);
	if (channel.is_user(sender) == false)
	{
		tmp = Sender::no_user_message(sender, sender.nickname_);
		ret.insert(tmp);
		return ret;
	}
	ret = channel.send_all(sender, sender, msg, PRIV);
	return ret;
}

/** @brief 채널 notice 처리 : privmsg와 본질적으로 같지만, notice라는 다른 규격 사용 **/
Udata	Database::notice_channel(User& sender, std::string chan_name, const std::string& msg)
{
	Udata			ret;
	Event			tmp;

	if (is_channel(chan_name) == false)
	{
		tmp = Sender::no_channel_message(sender, chan_name);
		ret.insert(tmp);
		return ret;
	}
	Channel&	channel = select_channel(chan_name);
	ret = channel.send_all(sender, sender, msg, NOTICE);
	return ret;
}

/** @brief kick 처리. 권한 확인, 실행, 통지 **/
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
	Channel&	channel = select_channel(chan_name);
	/** 권한 확인 **/
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
		tmp = Sender::kick_error_not_op_message(host, host.nickname_, chan_name);
		ret.insert(tmp);
	}
	return ret;
}

/**  @brief 유저의 리스트를 관리하고 있는 데이터베이스에 채널 구성원의 닉네임이 바뀌었음을 알려주어야 한다. **/
Udata	Database::nick_channel(User& nicker, std::string& send_msg)
{
	Udata		ret;
	Event		tmp;
	User		trash;

	Channel& channel = select_channel(nicker);
	ret = channel.send_all(nicker, trash, send_msg, NICK);
	channel.change_nick(nicker, send_msg);
	if (channel.get_host() == nicker)
	{
		channel.set_host(nicker);
	}
	return ret;
}

/** @brief 채널의 topic을 설정한다. 호스트만 가능하다. topic은 봇을 통해 확인할 수 있다. **/
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
	Channel& channel = select_channel(chan_name);
	if (channel.get_host() == sender)
	{
		std::string topic_msg = "Topic was changed to " + topic;
		channel.set_topic(topic);
		ret = channel.send_all(sender, sender, topic_msg, TOPIC);
	}
	else
	{
		std::string topic_msg = "You do not have the authority to change the topic on this channel";
		ret = channel.send_all(sender, sender, topic_msg, TOPIC);
	}
	return	ret;
}
