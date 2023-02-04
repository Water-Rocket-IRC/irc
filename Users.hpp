#ifndef USERS_HPP
# define USERS_HPP

// ident는 있는데 nick이 있는 지 확인해야함

#include "Sender.hpp"
#include "Udata.hpp"
#include "user.hpp"
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <exception>

/// @brief 유저들을 관리하고, sender에게 적절한 응답을 요청한다.
class Users
{
	private:
		std::vector<user>	user_list_;
	public:
		Event	command_nick(std::string& nick_name, const uintptr_t& ident);
		Event	command_user(const std::string input[4], const uintptr_t& ident);
		Event	command_quit(user& leaver, const std::string& leave_msg);
		Event	command_privmsg(std::string &target_name, std::string &line, const uintptr_t& ident);
		// Event	command_mode(std::string &target_name, std::string &line, const uintptr_t& ident);

		user&	search_user_by_ident(const uintptr_t& ident, int error_code);
		user&	search_user_by_nick(std::string nickname, int error_code);
		void	delete_user(user& leaver);
		bool 	is_duplicate_ident(const uintptr_t& ident);
		bool 	is_duplicate_nick(std::string& nick_name);

		bool	has_nick(const uintptr_t& ident);
		bool	has_username(const uintptr_t& ident);

		class no_user_found_exception : public std::exception
		{
			public:
				const char*	what(void) const throw();
		};
		class duplicated_user_found_exception : public std::exception
		{
			public:
				const char*	what(void) const throw();
		};
		void	print_all_user(); //debug
};

const char*	Users::no_user_found_exception::what(void) const throw()
{
	return "err: no user found";
}

const char*	Users::duplicated_user_found_exception::what(void) const throw()
{
	return "err: duplicated user found";
}

/// @brief 
// ident 즉 socket을 이용해 지금 명령어 친 user가 누군 지 알아낸다. 
user&	Users::search_user_by_ident(const uintptr_t& ident, int error_code)
{
	std::vector<user>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == ident) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	throw error_code;
	Sender::error_message(ident, "PRIVMSG", error_code);
	return (*it);
}

/// @brief 
// nick 이용해 user가 누군 지 알아낸다. <- 중복검사 활용
user&	Users::search_user_by_nick(std::string nickname, int error_code)
{
	std::vector<user>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	Sender::error_message(it->client_sock_, "NICK", error_code);
	return (*it);
}

bool	Users::is_duplicate_ident(const uintptr_t& ident)
{
	try
	{
		user&	tmp_user = search_user_by_ident(ident, 0); //duplicate일 때 에러 넘버를 체크
	}
	catch (const std::exception& e)
	{
		return false;
	}
	return true;
}

bool	Users::is_duplicate_nick(std::string& nick_name)
{
	try
	{
		user&	tmp_user = search_user_by_nick(nick_name, 0);
	}
	catch (std::exception&)
	{
		return false;
	}
	return true;
}

/// @brief 
// ident로 찾고, nick이 있는 지 확인 
bool	Users::has_nick(const uintptr_t& ident)
{
	try
	{
		user tmp = search_user_by_ident(ident, 0);	// No THROW
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
bool	Users::has_username(const uintptr_t& ident)
{
	try
	{
		user tmp = search_user_by_ident(ident, 0);	// No THROW
		if (tmp.username_.empty())
			return (false);
	}
	catch(const std::exception&)
	{
		return (false);
	}
	return (true);
}


void	Users::delete_user(user& leaver)
{
	std::vector<user>::iterator it;
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
Event	Users::command_privmsg(std::string &target_name, std::string &line, const uintptr_t& ident)
{
	Event	ret;
	user&	sender_user = search_user_by_ident(ident, 0);
	user&	target_user = search_user_by_nick(target_name, 0);

		
	// static Event	privmsg_p2p_message(const user& sender, const user& target, const std::string& msg);
	ret = Sender::privmsg_p2p_message(sender_user, target_user, line);
	return ret;
}

/// @brief
// quit을 실행하는 함수
Event	Users::command_quit(user& leaver, const std::string& leave_msg)
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
Event	Users::command_user(const std::string input[4], const uintptr_t& ident)
{
	Event	ret;

	try
	{
		user&	cur_user = search_user_by_ident(ident, 0);	// NO THROW
	}
	catch(const std::exception&)
	{
		user	tmp_user;
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
Event	Users::command_nick(std::string& new_nick, const uintptr_t& ident)
{
	Event		ret;
	int			error_code;

	// 새로 닉이 기존에 잇으면 433에러
	user&	cur_user = search_user_by_nick(new_nick, 433); 
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
		user		tmp_usr;
		tmp_usr.nickname_ = new_nick;
		tmp_usr.client_sock_ = ident;
		user_list_.push_back(tmp_usr);
	}
	return ret;
}


/// @brief 
// mode는 오류에 대한 것은 안 만들기로 함
// Event	command_mode(std::string &target_name, int flag)
// {
// 	Event	ret;

// 	ret = Sender::mode_well_message();
// 	return (ret);
// }

//debug 함수
void Users::print_all_user()
{
	std::vector<user>::iterator it;

	for (it = user_list_.begin(); it != user_list_.end(); ++it)
	{
		std::cout << "user " << it->nickname_ << std::endl; 
		std::cout << "socket " << it->client_sock_ << std::endl;
	}
}

#endif