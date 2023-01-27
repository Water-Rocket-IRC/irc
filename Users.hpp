#pragma once

#include <string>
#include <vector>

/// @brief 개별 클라이언트에서 접속한 유저의 정보
struct User
{
	std::string	nickname_;
	std::string username_;
	int			client_sock_;
	int			mod;
	//그 외 기타등등
};


/// @brief 유저들을 관리하고, sender에게 적절한 응답을 요청한다.
class Users
{
	private:
		std::vector<User> user_list_;
	public:
		Users();

};

Users::Users()
{
	
}