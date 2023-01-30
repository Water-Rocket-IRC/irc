#pragma once

#include "Chan.hpp"


class Channels
{
	private:
		std::vector<Chan> Channels_;

	public:
	//메시지를 전송하는 모든 명령은 std::vector<Udata>로 리턴할 것.
		std::vector<Udata> channel_msg(user& sender, std::string& msg);

		bool		is_channel(std::string& chan_name);
		void 		create_channel(user& joiner, std::string& chan_name);
		void 		delete_channel(std::string& chan_name);
		Chan&		select_channel(std::string& chan_name);

		std::vector<Udata> 	join_channel(user& joiner, std::string& chan_name);
		std::vector<Udata>	leave_channel(user&leaver, std::string& chan_name);

		std::vector<Chan>& get_channels() { return	Channels_; };
};

bool		Channels::is_channel(std::string& chan_name)
{
	std::vector<Chan>::iterator it;

	for (it = Channels_.begin(); it != Channels_.end(); ++it)
	{
		if (it->get_name() == chan_name)
		{
			return true;
		}
	}
	return false;
}

void Channels::create_channel(user& joiner, std::string& chan_name)
{
	Chan	tmp;
	
	tmp.set_channel_name(chan_name);
	tmp.add_user(joiner);
	tmp.set_host();
	Channels_.push_back(tmp);
}

 /* 
 * operator 가 없을 때 (0 index가 비어있다면 삭제)
 */
void Channels::delete_channel(std::string& chan_name)
{
	Chan tmp;

	tmp.set_channel_name(chan_name);
	std::vector<Chan>::iterator it = std::find(Channels_.begin(), \
	Channels_.end(), tmp);
	std::size_t size = std::distance(this->Channels_.begin(), it);
	this->Channels_.erase(this->Channels_.begin() + size);
}

Chan&	Channels::select_channel(std::string& chan_name)
{
	Chan tmp;

	tmp.set_channel_name(chan_name);
	std::vector<Chan>::iterator result = std::find(Channels_.begin(), \
	Channels_.end(), tmp);

	return *result;
}

std::vector<Udata> Channels::join_channel(user& joiner, std::string& chan_name)
{
	std::vector<Udata> msg;

	if (is_channel(chan_name) == false)
	{
		this->create_channel(joiner, chan_name);
	}
	else
	{
		Chan& chan = select_channel(chan_name);
		if (chan.is_user(joiner) == true)
		{
			std::cerr << "ERROR" << std::endl;
		}
		else
		{
			chan.add_user(joiner);
		}
	}

	//유저가 채널에 성공적으로 입장했다는 메시지 전송 + 서버 정보 전송
	return msg;
}

std::vector<Udata>	Channels::leave_channel(user&leaver, std::string& chan_name)
{
	if (is_channel(chan_name) == false)
	{
		//채널이 없을 경우, 오류 메시지 유저에게 전송, 이건 클라이언트를 통해 들어올 수 있음
	}
	else
	{
		Chan& chan = select_channel(chan_name);

		//유저가 존재하지 않을 경우(로비에서 part하면 예외처리)
		if (chan.is_user(leaver) == 0)
		{
			//유저에게 욕하는 메시지 전송
		}

		//Msg전송 : PART 내용에 따라 전송 -> 아마 채널의 다른 유저들에게 떠났다고 알려줘야
		std::vector<user> users = chan.get_users();

		//PART하면, 그 내역은 모두에게 보내진다. 나간 사람 포함한다.
        chan.send_all(sender, msg, 1);
		chan.delete_user(leaver);
		if (users.size() == 0)
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
	}
}

/// @brief 채널 전체 유저에게 메시지 전달. 내외부 모두 사용됨
std::vector<Udata> channel_msg(user& sender, std::string chan_name, std::string& msg)
{
	if (is_channel(chan_name) == false)
	{
		//채널이 없을 경우, 오류 메시지 유저에게 전송
		return ;
	}
	chan channel = select_channel(chan_name);

	//본인에겐 빼고 보내야함
	return channel.send_msg(sender, msg);
}
