#include "Database.hpp"
#include "Udata.hpp"
#include <locale>
#include <cctype>
#include <sys/_select.h>
#include "debug.hpp"

Event	Database::valid_user_checker_(const uintptr_t& ident, const std::string& command_type)
{
	Event	ret;

	if (!is_user(ident))
	{
		ret = Sender::command_not_registered_451(ident, command_type);
		return ret;
	}
	User&	cur_user = select_user(ident);
	if (!does_has_nickname(ident))
	{
		ret = Sender::command_not_registered_451(ident, command_type);
	}
	else if (!does_has_username(ident))
	{
		ret = Sender::command_not_registered_451(cur_user, command_type);
	}
	return ret;
}




/// @brief 
// ident 즉 socket을 이용해 지금 명령어 친 user가 누군 지 알아낸다. 
User&	Database::select_user(const uintptr_t& ident)
{
	std::vector<User>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == ident) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	throw no_such_user_exception();
	return (*it);
}

/// @brief
// nick 이용해 user가 누군 지 알아낸다. <- 중복검사 활용
User&	Database::select_user(const std::string& nickname)
{
	std::vector<User>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname) // 닉네임 바꿔야할 유저를 찾을 상태 !!!
		{
			return (*it);
		}
	}
	throw no_such_user_exception();
	return (*it);
}

// bool	Database::is_duplicate_ident(const uintptr_t& ident)
// {
// 	try
// 	{
// 		User&	tmp_user = search_user_by_ident(ident, 0); //duplicate일 때 에러 넘버를 체크
// 	}
// 	catch (const std::exception& e)
// 	{
// 		return false;
// 	}
// 	return true;
// }

// bool	Database::is_duplicate_nick(std::string& nick_name)
// {
// 	try
// 	{
// 		User&	tmp_user = search_user_by_nick(nick_name, 0);
// 	}
// 	catch (std::exception&)
// 	{
// 		return false;
// 	}
// 	return true;
// }

bool	Database::is_user(const uintptr_t& ident)
{
	try
	{
		User& tmp = select_user(ident);
	}
	catch(const std::exception&)
	{
		return (false);
	}
	return (true);
}

bool	Database::is_user(const std::string& nickname)
{
	try
	{
		User& tmp = select_user(nickname);
	}
	catch(const std::exception&)
	{
		return false;
	}
	return true;
}

/* @brief
 *		ident로 찾고, nick이 있는 지 확인
 */
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

/* @brief
 *		ident로 찾고, username이 있는 지 확인
 */
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



void	Database::delete_user(User& leaver)
{
	std::vector<User>::iterator it;
	int	idx = 0;

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

bool	Database::is_valid_nick(std::string& new_nick)
{
	for (int i = 0; i < new_nick.size(); i++)
	{
		if (!isalpha(new_nick[i]) && new_nick[i] != '_')
		{
			return false;
		}
	}
	return true;
}

/***************************************************************************************************/
Udata	Database::command_nick(const uintptr_t& ident, std::string& new_nick)
{
	Udata		ret;
	Event		tmp;

	tmp.first = ident;
	if (!is_valid_nick(new_nick)) // TODO: hchang 특수문자로 시작하는 닉네임 등 유효성 체크하는 함수 만들 것
	{
		if (!is_user(ident))
		{
			tmp = Sender::nick_wrong_message(ident, new_nick);
			ret.insert(tmp);
			return ret;
		}
		else
		{
			User&		you_usr = select_user(new_nick);
			tmp = Sender::nick_wrong_message(you_usr, new_nick);
			ret.insert(tmp);
			return ret;
		}
	}
	if (is_user(new_nick))// 닉네임 중복된 상황 433 에러
	{
		User&		you_usr = select_user(new_nick);
		// tmp = Sender::nick_error_message(you_usr, new_nick);
		if (ident == you_usr.client_sock_)
			return ret;
		if (you_usr.username_.size())
		{
			if (is_user(ident)) // 질문 : 198번줄과 중복 검사 ?
			{
				User&	cur_usr = select_user(ident);
				if (cur_usr.nickname_.empty())
					tmp = Sender::nick_error_message(ident, new_nick); // 433
				else
					tmp = Sender::nick_error_message(cur_usr, new_nick); // 432
			}
			else
			{
				tmp = Sender::nick_error_message(ident, new_nick);
			}
			ret.insert(tmp);
			return ret;
		}
		tmp = Sender::nick_error_message2(you_usr, new_nick);
		ret.insert(tmp);
		user_list_.erase(remove(user_list_.begin(), user_list_.end(), you_usr), user_list_.end()); // 순서 중요
	}
	if (!does_has_nickname(ident) && does_has_username(ident))
	{
		User&	cur_usr = select_user(ident);

		cur_usr.nickname_ = new_nick;
		tmp = Sender::welcome_message_connect(cur_usr);
	}
	else if (!is_user(ident))
	{
		User		tmp_usr;

		tmp_usr.client_sock_ = ident;
		tmp_usr.nickname_ = new_nick;
		user_list_.push_back(tmp_usr);
	}
	else// 기존 유저 닉네임 변경
	{
		User& cur_user = select_user(ident);
		if (! (does_has_nickname(ident) && !does_has_username(ident)) )
			tmp = Sender::nick_well_message(cur_user, cur_user, new_nick);
		cur_user.nickname_ = new_nick;
		//채널에 있지 않으니, 닉네임만 바꿈
		if (is_user_in_channel(cur_user)) // 채널에 있는 유저의 닉네임 변경
		{
			ret = nick_channel(cur_user, new_nick);
		}
	}
	ret.insert(tmp);
	debug::showUsers(user_list_);
	return ret;
}

Event	Database::command_user(const uintptr_t& ident
								, const std::string& username, const std::string& mode
								, const std::string& unused, const std::string& realname)
{
	Event	ret;

	ret.first = ident;

	std::cout << "=========[command_user]========\n";
	std::cout << "username : " << username << std::endl;
	std::cout << "mode : " << mode << std::endl;
	std::cout << "unuserd  : " << unused << std::endl;
	std::cout << "realname : " << realname << std::endl;
	std::cout << "=========[/command_user]========\n";



	if (does_has_nickname(ident) && !does_has_username(ident))
	{
		User&		cur_usr = select_user(ident);

		cur_usr.input_user(username, mode, unused, realname);
		ret = Sender::welcome_message_connect(cur_usr);;
	}
	else if (!is_user(ident))
	{
		User		tmp_usr;

		tmp_usr.client_sock_ = ident;
		tmp_usr.input_user(username, mode, unused, realname);
		user_list_.push_back(tmp_usr);
	}
	return ret;
}

Event	Database::command_pong(const uintptr_t& ident, const std::string& target, const std::string& msg)
{
	Event	ret;

	ret.first = ident;
	ret = valid_user_checker_(ident, "PING");
	if (ret.second.size())
		return ret;
	User&	cur_user = select_user(ident);	// user가 있는 경우
	if (msg.at(0) == ':')
	{
		ret = Sender::command_no_origin_specified_409(cur_user, "PING");
		return ret;
	}
	ret = Sender::pong(ident, target, msg);
	return ret;
}

Udata	Database::command_join(const uintptr_t& ident, const std::string& chan_name)
{
	Udata	ret;
	Event	tmp;

	tmp = valid_user_checker_(ident, "JOIN");
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
				tmp = Sender::command_empty_argument_461(ident, "JOIN");
			}
			else
			{
				tmp =  Sender::command_empty_argument_461(cur_usr, "JOIN");
			}
		}
		// else if (chan_name[0] == '#')
		// {
		// 	tmp = Sender::join_invaild_channel_name_message(cur_usr, chan_name);
		// }
		else
		{
			ret = join_channel(cur_usr, chan_name);
		}
	}
	ret.insert(tmp);
	return ret;
}

Udata	Database::command_quit(const uintptr_t& ident, const std::string& msg)
{
	Udata	ret;
	Event	tmp;

	tmp = valid_user_checker_(ident, "QUIT");
	if (tmp.second.size())
	{
		ret.insert(tmp);
		return ret;
	}
	else if (is_user(ident))
	{
		User&	cur_usr = select_user(ident);
		
		if (is_user_in_channel(cur_usr)) // 채널에 있으면  
		{
			// msg가 있으면 확인하고 분기점
			Channel&	cur_chan = select_channel(cur_usr);
			ret = leave_channel(cur_usr, cur_chan.get_name(), msg);
			return ret;
		}
		else // 채널에 없으면 (= 로비)
		{
			tmp = Sender::quit_leaver_message(cur_usr, msg);
			ret.insert(tmp);
		}
	}
	return ret;
}

/***************************************************************************************************/

/// @brief 
//	이 command에는 privmsg가 정상작동 될 때만 존재
Event	Database::command_privmsg(std::string &target_name, std::string &line, const uintptr_t& ident)
{
	Event		ret;
	// User&	sender_user = search_user_by_ident(ident, 0);
	// User&	target_user = search_user_by_nick(target_name, 0);

		
	// static Event	privmsg_p2p_message(const User& sender, const User& target, const std::string& msg);
	// ret = Sender::privmsg_p2p_message(sender_user, target_user, line);
	return ret;
}

/// @brief
// quit을 실행하는 함수
// Event	Database::command_quit(User& leaver, const std::string& leave_msg)
// {
// 	Event	ret;

// 	delete_user(leaver);
// 	if (leave_msg.empty())
// 	{
// 		ret = Sender::quit_lobby_message(leaver, "");
// 		return ret;
// 	}
// 	ret = Sender::quit_lobby_message(leaver, leave_msg);
// 	return ret;
// }

/// @brief
// user를 실행하는 함수 (command_user는 Event 반환)

/// @brief 
// nick을 실행하는 함복


/// @brief 
// mode는 오류에 대한 것은 안 만들기로 함
Event	Database::command_mode(std::string &target_name, int flag)
{
	Event	ret;

	try 
	{
		// User&	sender = search_user_by_nick(target_name, 0);
		// ret = Sender::connect_mode_message(sender);
		return (ret);
	}
	catch(std::exception& e)
	{
		return (ret);
	}
	return (ret);
}

//debug 함수
void Database::print_all_user()
{
	std::vector<User>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); ++it)
	{
		std::cout << "user " << it->nickname_ << std::endl; 
		std::cout << "socket " << it->client_sock_ << std::endl;
	}
}
