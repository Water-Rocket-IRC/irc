#include "Database.hpp"
#include "Udata.hpp"
#include "User.hpp"
#include <string>
#include <sys/_types/_ct_rune_t.h>

std::vector<User>&	Database::get_user_list(void)
{
	return user_list_;
}

void	Database::delete_error_user(const uintptr_t& ident)
{
	if (!is_user(ident))
	{
		return ;
	}
	User&		cur_usr = select_user(ident);
	if (is_user_in_channel(cur_usr))
	{
		Channel& cur_chan = select_channel(cur_usr);

		std::vector<User>& users = cur_chan.get_users();
		const int user_size = users.size();
		cur_chan.delete_user(cur_usr);
		if (user_size == 1)
		{
			delete_channel(cur_chan.get_name());
		}
		else
		{
			if (cur_usr == cur_chan.get_host())
			{
				cur_chan.set_host();
			}
		}
	}
	user_list_.erase(remove(user_list_.begin(), user_list_.end(), cur_usr), user_list_.end());
}

Event	Database::valid_user_checker_(const uintptr_t& ident, const std::string& command_type)
{
	Event	ret;

	ret.first = ident;
	if (!is_user(ident))
	{
		return Sender::password_incorrect_464(ident);
	}
	User&	cur_user = select_user(ident);
	if (!(cur_user.flag_ & F_PASS))
	{
		return Sender::password_incorrect_464(ident);
	}
	if (command_type == "NICK" || command_type == "USER")
	{
		return ret;
	}
	if (!(cur_user.flag_ & F_NICK))
	{
		return Sender::command_not_registered_451(ident, command_type);
	}
	if (!(cur_user.flag_ & F_USER))
	{
		return Sender::command_not_registered_451(cur_user, command_type);
	}
	return ret;
}

/** @brief
ident 즉 socket을 이용해 지금 명령어 친 user가 누군 지 알아낸다. **/
User&	Database::select_user(const uintptr_t& ident)
{
	std::vector<User>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == ident)
		{
			return (*it);
		}
	}
	return (*it);
}

/** @brief
nick 이용해 user가 누군 지 알아낸다. **/
User&	Database::select_user(const std::string& nickname)
{
	std::vector<User>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname)
		{
			return (*it);
		}
	}
	return (*it);
}

/** @brief
유저가 있는 지 ident를 통해 검사를 한다. **/
bool	Database::is_user(const uintptr_t& ident)
{
	for (std::vector<User>::iterator it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == ident)
		{
			return (true);
		}
	}
	return (false);
}

/** @brief
유저가 있는 지 nickname를 통해 검사를 한다. **/
bool	Database::is_user(const std::string& nickname)
{
	for (std::vector<User>::iterator it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname)
		{
			return (true);
		}
	}
	return (false);
}

/** @brief
ident로 유저가 있는 지 검사를 하고, 있으면 nickname은 있는 지 체크한다. **/
bool	Database::does_has_nickname(const uintptr_t& ident)
{
	try
	{
		User& tmp = select_user(ident);
		if (tmp.nickname_.empty())
			return (false);
	}
	catch(const std::exception&)
	{
		return (false);
	}
	return (true);
}

/** @brief
ident로 유저가 있는 지 검사를 하고, 있으면 username은 있는 지 체크한다. **/
bool	Database::does_has_username(const uintptr_t& ident)
{
	try
	{
		User& tmp = select_user(ident);
		if (tmp.username_.empty())
			return (false);
	}
	catch(const std::exception&)
	{
		return (false);
	}
	return (true);
}

/** @brief
지금 떠난 leaver가 있는 지 체크한 후 user_list_에서 지워준다. **/
void	Database::delete_user(User& leaver)
{
	int	idx = 0;
	std::vector<User>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (*it == leaver)
		{
			user_list_.erase(user_list_.begin() + idx);
			break ;
		}
		idx++;
	}
}

/** @brief
새로 작성한 new_nick이 유효한 nick인 지 체크 **/
bool	Database::is_valid_nick(std::string& new_nick)
{
	if (!isalpha(static_cast<int>(new_nick[0])) && static_cast<int>(new_nick[0]) != '_')
		return false;
	for (std::size_t i(1); i < new_nick.size(); i++)
	{
		if (!isalnum(static_cast<int>(new_nick[i])) && static_cast<int>(new_nick[i]) != '_')
		{
			return false;
		}
	}
	return true;
}

/******************************************[command part]******************************************/

/** @brief
command_pass : user가 없으면 등록해준다. **/
Event	Database::command_pass(const uintptr_t& ident)
{
	Event	tmp;

	tmp.first = ident;
	if (!is_user(ident))
	{
		User	tmp_user;
		tmp_user.client_sock_ = ident;
		tmp_user.flag_ |= F_PASS;
		user_list_.push_back(tmp_user);
	}
	return tmp;
}

/** @brief
command_nick :  유효성 검사 / 중복 검사 / join 전 후 nick을 한 경우를 다 한 후, nick을 변경해준다. **/
Udata	Database::command_nick(const uintptr_t& ident, std::string& new_nick)
{
	Udata		ret;
	Event		tmp = valid_user_checker_(ident, "NICK");

	/** 유효성 검사**/
	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	User&	cur_usr = select_user(ident);
	/** 새로운 nickname '유효성' 검사 **/
	if (!is_valid_nick(new_nick))
	{
		if (cur_usr.nickname_.empty())
			tmp = Sender::nick_wrong_message(ident, new_nick);
		else
		{
			User&	you_usr = select_user(cur_usr.nickname_);
			tmp = Sender::nick_wrong_message(you_usr, new_nick);
		}
		ret.insert(tmp);
		return ret;
	}
	/** 새로운 nickname '중복' 검사 **/
	if (is_user(new_nick))
	{
		User&		you_usr = select_user(new_nick);
		if (ident == you_usr.client_sock_)
			return ret;
		if (you_usr.username_.size())
		{
			if (cur_usr.nickname_.empty())
				tmp = Sender::nick_error_message(ident, new_nick); /** 아직 join이 되지 않았을 떄 중복되면, 433 뱉어준다. **/
			else
				tmp = Sender::nick_error_message(cur_usr, new_nick); /** ㅓoin이 된 상태일 떄 중복되면, 432 뱉어준다. **/
			ret.insert(tmp);
			return ret;
		}
		tmp = Sender::nick_error_message2(you_usr, new_nick);
		ret.insert(tmp);
		user_list_.erase(remove(user_list_.begin(), user_list_.end(), you_usr), user_list_.end()); // 순서 중요
	}
	/** 처음으로 join되기 전에 nick 명령어를 실행한 경우 **/
	if (!(cur_usr.flag_ & F_NICK))
	{
		cur_usr.nickname_ = new_nick;
		cur_usr.flag_ |= F_NICK;
		if (cur_usr.flag_ & F_USER) /** user 명령어를 이미 해본 상태에서 nick이 되면 join된 상태이기에 welcome_message를 뱉어준다. **/
		{
			tmp = Sender::welcome_message_connect(cur_usr);
			ret.insert(tmp);
		}
	}
	else /** 기존 유저가 nick을 바꾸려는 상태 **/
	{
		/** 채널에도 있는 상태라면 channel_list_의 유저 nick도 변경해줘야한다. **/
		if (is_user_in_channel(cur_usr))
		{
			ret = nick_channel(cur_usr, new_nick);
		}
		tmp = Sender::nick_well_message(cur_usr, cur_usr, new_nick);
		cur_usr.nickname_ = new_nick;
		ret.insert(tmp);
	}
	return ret;
}

/** @brief
command_user : 처음에 nickname, username, host, realname 지정 **/
Event	Database::command_user(const uintptr_t& ident
								, const std::string& username, const std::string& mode
								, const std::string& unused, const std::string& realname)
{
	Event	ret = valid_user_checker_(ident, "USER");

	/** 유효성 검사**/
	if (ret.second.size())
	{
		return ret;
	}
	User&	cur_usr = select_user(ident);
	/** 처음으로 join되기 전에 user 명령어를 실행한 경우 **/
	if (!(cur_usr.flag_ & F_USER))
	{
		cur_usr.flag_ |= F_USER;
		cur_usr.input_user(username, mode, unused, realname);
		if (cur_usr.flag_ & F_NICK) /** nick 명령어를 이미 해본 상태에서 user가 되면 join된 상태이기에 welcome_message를 뱉어준다. **/
		{
			ret = Sender::welcome_message_connect(cur_usr); 
		} 
	}
	return ret;
}

/** @brief
command_pong : ping에 대한 pong **/
Event	Database::command_pong(const uintptr_t& ident, const std::string& target, const std::string& msg)
{
	Event	ret = valid_user_checker_(ident, "PING"); 

	/** 유효성 검사**/
	if (ret.second.size())
	{
		return ret;
	}
	User&	cur_user = select_user(ident);
	if (msg.at(0) == ':')
	{
		ret = Sender::command_no_origin_specified_409(cur_user, "PING");
		return ret;
	}
	ret = Sender::pong(ident, target, msg);
	return ret;
}

/** @brief
command_join : 채널에 join을 한다. **/
Udata	Database::command_join(const uintptr_t& ident, const std::string& chan_name)
{
	Udata	ret;
	Event	tmp = valid_user_checker_(ident, "JOIN");

	/** 유효성 검사**/
	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	else if (is_user(ident))
	{
		User&	cur_usr = select_user(ident);
		if (chan_name.empty()) 
		{
			if (!is_user(ident))
			{
				tmp = Sender::command_empty_argument_461(ident, "JOIN"); /** 명렁어 join에서 충분한 명령어가 오지 않았다. **/
				ret.insert(tmp);
			}
			else
			{
				tmp =  Sender::command_empty_argument_461(cur_usr, "JOIN"); /** 명렁어 join에서 충분한 명령어가 오지 않았다. **/
				ret.insert(tmp);
			}
		}
		else if (chan_name.at(0) != '#')
		{
			tmp = Sender::join_invaild_channel_name_message(cur_usr, chan_name);
			ret.insert(tmp);
		}
		else
		{
			ret = join_channel(cur_usr, chan_name);
		}
	}
	return ret;
}

/** @brief
command_join : 채널에서 나가는 명령어. **/
Udata	Database::command_part(const uintptr_t& ident, std::string& chan_name, const std::string& msg)
{
	Udata	ret;
	Event	tmp = valid_user_checker_(ident, "PART");

	/** 유효성 검사**/
	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	User& cur_usr = select_user(ident);
	ret = part_channel(cur_usr, chan_name, msg);
	return ret;
}

/** @brief
command_join : 채널에서 나가는 명령어. **/
Udata	Database::command_quit(const uintptr_t& ident, const std::string& msg)
{
	Udata	ret;
	Event	tmp = valid_user_checker_(ident, "QUIT");

	/** 유효성 검사**/
	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	else if (is_user(ident))
	{
		User&	cur_usr = select_user(ident);

		if (is_user_in_channel(cur_usr)) /** 만약 채널에 있는 상태라면 **/
		{
			Channel&	cur_chan = select_channel(cur_usr);
			ret = quit_channel(cur_usr, cur_chan.get_name(), msg);
		}
		else /** 만약 채널에 없는 상태라면 (= 로비) **/
		{
			tmp = Sender::quit_leaver_message(cur_usr, msg);
			ret.insert(tmp);
		}
		user_list_.erase(remove(user_list_.begin(), user_list_.end(), cur_usr), user_list_.end());
	}
	return ret;
}

/** @brief
command_privmsg : 유저와 채널에 privmsg할 수 있는 명령어 **/
Udata	Database::command_privmsg(const uintptr_t& ident, const std::string &target_name, const std::string &msg)
{
	Udata		ret;
	Event		tmp = valid_user_checker_(ident, "PRIVMSG");
	
	/** 유효성 검사**/
	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	if (is_user(ident))
	{
		User&	cur_usr = select_user(ident);

		tmp.first = ident;
		if (target_name.at(0) == '#') /** 채널에 보낼 때 (내가 channel에 존재해야한다.) **/
		{
			ret = channel_msg(cur_usr, target_name, msg);
		}
		else
		{
			if (target_name == "BOT") /** BOT을 불러내는 명령어 **/
			{
				tmp = bot_privmsg(cur_usr, msg);
				ret.insert(tmp);
			}
			else if (is_user(target_name)) /** 유저가 있을 때 그 유저에게 privmsg를 한다. **/
			{
				User&	tar_usr = select_user(target_name);
				tmp = Sender::privmsg_p2p_message(cur_usr, tar_usr, msg);
				ret.insert(tmp);
			}
			else /** 보내려는 유저가 없을 때 **/
			{
				tmp = Sender::privmsg_no_user_error_message(cur_usr, target_name);
				ret.insert(tmp);
			}
		}
	}
	return ret;
}

/** @brief
command_notice : 유저와 채널에 notice할 수 있는 명령어 **/
Udata	Database::command_notice(const uintptr_t& ident, const std::string &target_name, const std::string &msg)
{
	Udata		ret;
	Event		tmp = valid_user_checker_(ident, "NOTICE"); 

	/** 유효성 검사**/
	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	if (target_name == "BOT")
	{
		ret.insert(tmp);
		return ret;
	}
	User&	cur_usr = select_user(ident);
	if (target_name.at(0) == '#')
	{
		ret = notice_channel(cur_usr, target_name, msg);
	}
	else
	{
		if (is_user(target_name)) /** 유저가 있을 때 그 유저에게 notice를 한다. **/
		{
			User&	tar_usr = select_user(target_name);

			tmp = Sender::notice_p2p_message(cur_usr, tar_usr, msg);
			ret.insert(tmp);
		}
		else /** 보내려는 유저가 없을 때 **/
		{
			tmp = Sender::notice_no_nick_message(cur_usr, cur_usr);
			ret.insert(tmp);
		}
	}
	return ret;
}

/** @brief
command_kick : 채널에서 user를 방출하는 명렁어 (channel host만 가능) **/
Udata		Database::command_kick(const uintptr_t &ident, const std::string& target_name, std::string& chan_name, std::string& msg)
{
	Event	tmp = valid_user_checker_(ident, "KICK");
	Udata	ret;

	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	if (is_user(ident))  /** 유저가 있을 때 그 유저를 kick한다. **/
	{
		User& kicker = select_user(ident);
		if (is_user(target_name))
		{
			User& target = select_user(target_name);
			ret = kick_channel(kicker, target, chan_name, msg);
		}
		else /** 내보내려는 유저가 없을 때 **/
		{
			tmp = Sender::no_user_message(kicker, target_name);
			ret.insert(tmp);
		}
	}
	return ret;
}

/******************************************[bot part]******************************************/

/** @brief
초반에 유저로 bot을 만든다. **/
void	Database::bot_maker(const std::string& name)
{

	User		tmp_usr;

	tmp_usr.nickname_ = name;
	tmp_usr.input_user("Dummy", "Dummy", "localhost", "Dummy");
	user_list_.push_back(tmp_usr);
}

/** @brief
bot이 뱉어 줄 message 구성 **/
Event	Database::bot_privmsg(User&	cur_usr, const std::string &msg)
{
	Event		tmp;
	std::string bot_msg;

	if (msg == "!command")
	{
		bot_msg = "NICK, USER, PING, JOIN, QUIT, PRIVMSG, KICK, PART, TOPIC, NOTICE";
	}
	else if (msg == "!channel")
	{
		if (channel_list_.empty())
		{
			bot_msg = "NO CHANNEL IN THIS SERVER!";
		}
		else
		{
			bot_msg = "● [CHANNEL LIST] : ";
			for (std::size_t i(0); i < channel_list_.size(); ++i)
			{
				bot_msg += std::to_string(i + 1) + ". " + channel_list_[i].get_name() + " : " + channel_list_[i].get_topic() + ((i == (channel_list_.size() - 1)) ? "": ", ");
			}
		}
	}
	else if (msg == "!user")
	{
		bot_msg = "● [USER LIST] : ";
		for (std::size_t i(1); i < user_list_.size(); ++i)
			bot_msg += std::to_string(i) + ". " + user_list_[i].nickname_ + ((i == (user_list_.size() - 1)) ? "": ", ");
	}
	else
	{
		bot_msg = "THAT IS NOT MY COMMAND. YOU CAN USE : '!command' & '!channel' & !user.";
	}
	tmp = Sender::privmsg_bot_message(cur_usr, bot_msg);
	return tmp;
}
