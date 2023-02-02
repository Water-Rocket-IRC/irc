#ifndef USERS_HPP
# define USERS_HPP

#include "Sender.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <sys/event.h>
#include <sys/time.h>
#include <exception>

// struct user;

/// @brief 유저들을 관리하고, sender에게 적절한 응답을 요청한다.
class Users
{
	private:
		std::vector<struct user>	user_list_;
	public:
		Udata	command_nick(std::string& nick_name, uintptr_t &sock);
		Udata	command_user(const std::string input[4], uintptr_t& sock);
		Udata	command_quit(std::stringstream &line_ss, uintptr_t& sock);
		Udata	command_privmsg(std::stringstream &line_ss, std::string &line, uintptr_t& sock);

		user&	search_user_by_ident(uintptr_t& sock);
		user&	search_user_by_nick(std::string nickname);
		void	delete_user(user& leaver);
		bool 	is_duplicate_ident(uintptr_t& sock);
		bool 	is_duplicate_nick(std::string& nick_name);
		void	change_nickname(user& cur, std::string& nick_name);

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


// ident 즉 socket을 이용해 지금 명령어 친 user가 누군 지 알아낸다. 
user&	Users::search_user_by_ident(uintptr_t& sock)
{
	std::vector<user>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->client_sock_ == sock) //140732838809008 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	throw no_user_found_exception();
	return (*it);
}

// nick 이용해 user가 누군 지 알아낸다. <- 중복검사 활용
user&	Users::search_user_by_nick(std::string nickname)
{
	std::vector<user>::iterator	it;

	for (it = user_list_.begin(); it != user_list_.end(); it++)
	{
		if (it->nickname_ == nickname) // 닉네임 바꿔야할 유저를 찾을 상태 !!! 
		{
			return (*it);
		}
	}
	throw no_user_found_exception();
	return (*it);
}

bool	Users::is_duplicate_ident(uintptr_t& sock)
{
	try
	{
		user&	tmp_user = search_user_by_ident(sock);
	}
	catch (std::exception& e)
	{
		return false;
	}
	return true;
}

bool	Users::is_duplicate_nick(std::string& nick_name)
{
	try
	{
		user&	tmp_user = search_user_by_nick(nick_name);
	}
	catch (std::exception& e)
	{
		return false;
	}
	return true;
}

void	Users::change_nickname(user& cur, std::string& new_nick_name)
{	
	cur.nickname_ = new_nick_name;
}



// nick을 실행하는 함수
// nick 처음  두번 째
Udata	Users::command_nick(std::string& nick_name, uintptr_t& sock)
{
	Udata		tmp;

	bzero(&tmp, sizeof(tmp));

	if (is_duplicate_nick(nick_name))									// 중복 체크
	{
		try
		{
			user &cur_user = search_user_by_ident(sock);				// 처음 소켓인지 
			tmp = Sender::nick_error_message(cur_user, nick_name);		// 기존에 유저가 없는 상태에서 중복 에러
		}
		catch (std::exception&)
		{
			tmp = Sender::nick_error_message(nick_name, sock);			// 기존에 유저가 없는 상태에서 중복 에러
		}
		return tmp;
	}
	try
	{
		user &cur_user = search_user_by_ident(sock);					// who?
		if (nick_name.size() > 1 && nick_name.at(0) == '#')
		{
			tmp = Sender::nick_wrong_message(cur_user, nick_name);
		}
		else 
		{
			tmp = Sender::nick_well_message(cur_user, cur_user, nick_name);
			cur_user.nickname_ = nick_name;
		}
		return tmp;
	}
	catch (std::exception& e)
	{
		user	tmp_usr;
		tmp_usr.nickname_ = nick_name;
		tmp_usr.client_sock_ = sock;
		user_list_.push_back(tmp_usr);
		return tmp;
	}
	return tmp;
}

// user를 실행하는 함수 
Udata	Users::command_user(const std::string input[4], uintptr_t& sock)
{
	Udata		tmp;

	bzero(&tmp, sizeof(tmp));
	try
	{
		user&	cur_user = search_user_by_ident(sock);

		if (cur_user.username_.empty())
		{
			cur_user.username_ = input[0];
			cur_user.mode_ = input[1];
			cur_user.unused_ = input[2];
			cur_user.realname_ = input[3].substr(1);
			return Sender::welcome_message_connect(cur_user);
		}
		return tmp;
	}
	catch (std::exception& e)
	{
		return tmp;
	}
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

// quit을 실행하는 함수
Udata	Users::command_quit(std::stringstream &line_ss, uintptr_t& sock)
{
	Udata		ret;
	user		leaver;
	std::string	leave_msg;

	line_ss >> leave_msg;
	leaver = search_user_by_ident(sock);
	delete_user(leaver);
	ret = Sender::quit_lobby_message(leaver, leave_msg);

	return ret;
}

Udata	Users::command_privmsg(std::stringstream &line_ss, std::string &line, uintptr_t& sock)
{
	std::string nick, msg;
	user	sender, target;
	Udata	ret;
	try
	{
		line_ss >> nick >> msg;
		std::cout << sock << std::endl;
		sender = search_user_by_ident(sock);
		try
		{
			target = search_user_by_nick(nick);
		}
		catch (std::exception& e())
		{
			return Sender::privmsg_no_user_error_message(sender, nick);
		}
		size_t	pos = line.find(':');
		msg = line.substr(pos + 1, (line.length() - (pos + 2)));
		ret = Sender::privmsg_p2p_message(sender, target, msg);
	}
	catch (std::exception &e)
	{
		std::cout << "No try" << std::endl;
	}

	return ret;
}

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