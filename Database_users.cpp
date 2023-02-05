#include "Database.hpp"

/// @brief 
// ident 즉 socket을 이용해 지금 명령어 친 user가 누군 지 알아낸다. 
User&	Database::search_user_by_ident(const uintptr_t& ident, int error_code)
{
	std::vector<User>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == ident) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	throw error_code;
	// Sender::error_message(ident, "PRIVMSG", error_code);
	return (*it);
}

/// @brief 
// nick 이용해 user가 누군 지 알아낸다. <- 중복검사 활용
User&	Database::search_user_by_nick(std::string nickname, int error_code)
{
	std::vector<User>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	// Sender::error_message(it->client_sock_, "NICK", error_code);
	return (*it);
}

bool	Database::is_duplicate_ident(const uintptr_t& ident)
{
	try
	{
		User&	tmp_user = search_user_by_ident(ident, 0); //duplicate일 때 에러 넘버를 체크
	}
	catch (const std::exception& e)
	{
		return false;
	}
	return true;
}

bool	Database::is_duplicate_nick(std::string& nick_name)
{
	try
	{
		User&	tmp_user = search_user_by_nick(nick_name, 0);
	}
	catch (std::exception&)
	{
		return false;
	}
	return true;
}

/// @brief 
// ident로 찾고, nick이 있는 지 확인 
bool	Database::has_nick(const uintptr_t& ident)
{
	try
	{
		User tmp = search_user_by_ident(ident, 0);	// No THROW
		if (tmp.nickname_.empty())
			return (false);
	}
	catch(const std::exception&)
	{
		return (false);
	}
	return (true);
}

/// @brief 
// ident로 찾고, username이 있는 지 확인 
bool	Database::has_username(const uintptr_t& ident)
{
	try
	{
		User tmp = search_user_by_ident(ident, 0);	// No THROW
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

/// @brief 
//	이 command에는 privmsg가 정상작동 될 때만 존재
Event	Database::command_privmsg(std::string &target_name, std::string &line, const uintptr_t& ident)
{
	Event		ret;
	User&	sender_user = search_user_by_ident(ident, 0);
	User&	target_user = search_user_by_nick(target_name, 0);

		
	// static Event	privmsg_p2p_message(const User& sender, const User& target, const std::string& msg);
	ret = Sender::privmsg_p2p_message(sender_user, target_user, line);
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
		User&	cur_user = search_user_by_ident(ident, 0);	// NO THROW
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
Event	Database::command_nick(std::string& new_nick, const uintptr_t& ident)
{
	Event		ret;
	int			error_code;

	// 새로 닉이 기존에 있으면 433에러
	User&	cur_user = search_user_by_nick(new_nick, 433); 
	try
	{
		// 기존 유저 닉네임 변경
		cur_user = search_user_by_ident(ident, 0);
		cur_user.nickname_ = new_nick;
		ret = Sender::nick_well_message(cur_user, cur_user, new_nick);
	}
	catch(std::exception& e)
	{
		// 처음 접속 닉네임 설정
		User		tmp_usr;
		tmp_usr.nickname_ = new_nick;
		tmp_usr.client_sock_ = ident;
		user_list_.push_back(tmp_usr);
	}
	return ret;
}


/// @brief 
// mode는 오류에 대한 것은 안 만들기로 함
Event	Database::command_mode(std::string &target_name, int flag)
{
	Event	ret;

	try 
	{
		User&	sender = search_user_by_nick(target_name, 0);
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
