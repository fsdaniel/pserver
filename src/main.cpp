#include <cstdlib>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>

#include "server.hpp"

int main(int argc, char **argv)
{
	srand(time(nullptr));
	
	try
	{
		boost::asio::io_service io;
		tcp::endpoint ep(tcp::v4(), 9998);
		Server s(io, ep);
		io.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "Exception caught: " << e.what() << '\n';
	}
	
	return 0;
}
