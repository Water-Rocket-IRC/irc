#include <ctime>
#include <iostream>
 
int main()
{
    std::time_t result = std::time(NULL);
    std::string str = ctime(&result);
    std::cout << str << " seconds since the Epoch\n";
}