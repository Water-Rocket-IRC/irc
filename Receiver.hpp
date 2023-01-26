#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include <string>

class Receiver
{
	private:
		std::string port;
		
	public:
		Receiver();
		~Receiver();
		void init();
		void start();
};

Receiver::Receiver()
{

}

Receiver::~Receiver()
{

}

void Receiver::init()
{

}

void Receiver::start()
{
	while (1)
	{
		
	}
}

#endif