#include "Database.hpp"
#include <locale>

// void	Database::valid_user_checker_(const uintptr_t& ident, const std::string& command_type)
// {
// 	try
// 	{
// 		user&	cur_user = users_.search_user_by_ident(ident, 0);
// 		if (!users_.has_nick(ident))
// 		{
// 			throw Sender::command_not_registered_451(ident, command_type);
// 		}
// 		else if (!users_.has_username(ident))
// 		{
// 			throw Sender::command_not_registered_451(cur_user, command_type);
// 		}
// 	}
// 	catch (std::exception& e) { }
// }




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
		return (false);
	}
	return (true);
}

/// @brief 
// ident로 찾고, nick이 있는 지 확인 
// bool	Database::dose_has_nick(const uintptr_t& ident)
// {
// 	try
// 	{
// 		User tmp = select_user(ident, 0);	// No THROW
// 		if (tmp.nickname_.empty())
// 			return (false);
// 	}
// 	catch(const std::exception&)
// 	{
// 		return (false);
// 	}
// 	return (true);
// }

// /// @brief 
// // ident로 찾고, username이 있는 지 확인 
// bool	Database::does_has_username(const uintptr_t& ident)
// {
// 	try
// 	{
// 		User tmp = select_user(ident, 0);	// No THROW
// 		if (tmp.username_.empty())
// 			return (false);
// 	}
// 	catch(const std::exception&)
// 	{
// 		return (false);
// 	}
// 	return (true);
// }



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

/***************************************************************************************************/
Udata	Database::command_nick(const uintptr_t& ident, std::string& new_nick)
{
	Udata		ret;
	Event		tmp;

	// if (is_valid_nick(new_nick)) // TODO: hchang 특수문자로 시작하는 닉네임 등 유효성 체크하는 함수 만들 것
	// {
			// 432 eeror
	// }

	if (is_user(new_nick)) // 닉네임 중복된 상황 433 에러
	{
		tmp = Sender::nick_error_message(ident, new_nick);
		ret.insert(tmp);
	}
	else if (!is_user(ident)) // 처음 접속한 경우
	{
		User		tmp_usr;

		tmp_usr.nickname_ = new_nick;
		tmp_usr.client_sock_ = ident;
		user_list_.push_back(tmp_usr);
		ret.insert(tmp);
	}
	else // 기존 유저 닉네임 변경
	{
		User& cur_user = select_user(ident);
		cur_user.nickname_ = new_nick;
		tmp = Sender::nick_well_message(cur_user, cur_user, new_nick);
		//채널에 있지 않으니, 닉네임만 바꿈
		if (is_user_in_channel(cur_user)) // 채널에 있는 유저의 닉네임 변경
		{
			ret = nick_channel(cur_user, new_nick);
		}
		ret.insert(tmp);
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
Event	Database::command_quit(User& leaver, const std::string& leave_msg)
{
	Event	ret;

	delete_user(leaver);
	if (leave_msg.empty())
	{
		ret = Sender::quit_lobby_message(leaver, "");
		return ret;
	}
	ret = Sender::quit_lobby_message(leaver, leave_msg);
	return ret;
}

/// @brief
// user를 실행하는 함수 (command_user는 Event 반환)
Event	Database::command_user(const std::string input[4], const uintptr_t& ident)
{
	Event	ret;

	try
	{
		// User&	cur_user = search_user_by_ident(ident, 0);	// NO THROW
	}
	catch(const std::exception&)
	{
		User	tmp_user;
		tmp_user.username_ = input[0];
		tmp_user.mode_ = input[1];
		tmp_user.unused_ = input[2];
		tmp_user.realname_ = input[3].substr(1);
		tmp_user.client_sock_ = ident;
		ret = Sender::welcome_message_connect(tmp_user);
	}
	return (ret);
}

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
