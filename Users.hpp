#pragma once

#include "Sender.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

enum mod
{
	NORMAL,
	ADMIN
};

/// @brief 개별 클라이언트에서 접속한 유저의 정보
struct user
{
	std::string	nickname_;
	std::string username_;
	std::string hostname_;
	std::string servername_;
	std::string realname_;

	int			client_sock_;
	int			mod;
	//그 외 기타등등
};


/// @brief 유저들을 관리하고, sender에게 적절한 응답을 요청한다.
class Users
{
	private:
		std::vector<user> user_list_;
	public:
		Users();
		void addnick(std::stringstream &line_ss, uintptr_t sock);
		void adduser(std::stringstream &line_ss, uintptr_t sock);

};

Users::Users()
{
	
}

void	Users::addnick(std::stringstream &line_ss, uintptr_t sock)
{
	user tmp_usr;
	std::string nickname;
	std::vector<user>::iterator it;
	bool flag = true;

	line_ss >> nickname;

	 for (it = user_list_.begin(); it != user_list_.end(); ++it)
	 {
		//client_sock_이 중복되지 않을 것이란건 내 뇌피셜. sesim 이거 보면 확인해줄 것.
		if (it->nickname_ == nickname || it->client_sock_ == sock)
		{
			//sender의 에러메시지 메소드 호출
			flag = false;
		}
	 }
	if (flag == true)
	{
		tmp_usr.nickname_ = nickname;
		tmp_usr.client_sock_ = (int)sock;
		user_list_.push_back(tmp_usr);
	}
}

/*
https://datatracker.ietf.org/doc/html/rfc1459#section-4.1.3
에 따르면, 이건 서버에 처음 접속할때 사용되는 명령어다. NICK과 USER 모두 접수되어야, 서버에 레지스터 된 것이다.
중간에 유저 명령어 전송이 가능한지 확인해야 하고, 그에 따라 처리해야 한다.
*/
void Users::adduser(std::stringstream &line_ss, uintptr_t sock)
{
	std::vector<user>::iterator it;
	user tmp_user;

	 for (it = user_list_.begin(); it != user_list_.end(); ++it)
	 {
		//접속한 소켓을 찾아 정보를 추가한다
		if (it->client_sock_ == sock)
		{
			tmp_user = *it;

			if (tmp_user.nickname_.empty())
				break;
			std::string username, hostname, servername, realname;
			line_ss >> username >> hostname >> servername >> realname;
			realname.erase(0, 1); //prefix 제거

			tmp_user.username_ = username;
			tmp_user.hostname_ = hostname;
			tmp_user.servername_ = servername;
			tmp_user.realname_ = realname;

			//for debug
			std::cout << "user " << username << std::endl; 
			std::cout << "host " << hostname << std::endl;
			std::cout << "server " << servername << std::endl;
			std::cout << "real " << realname << std::endl;

			Sender::welcome_message(it->client_sock_, it->servername_);
			break;
		}

		// nick없이 user만 들어왔으면 sender로 에러 메시지 출력? 실제 클라이언트와 서버가 어떻게 행동하는지 살펴보고 행동 결정해야함
	 }


}
