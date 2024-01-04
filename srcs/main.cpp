#include "Server.hpp"


DBConn	*db;

__attribute__((constructor)) void setup()
{
	db = nullptr;
	db = new DBConn(DB_HOST, DB_PORT, DB_USER, DB_PASS, DB_NAME);
}

__attribute__((destructor)) void teardown()
{
	if (db->status == 0)
		delete db;
}

int	main(int argc, char **argv)
{
	try
	{
		if (argc != 3)
		{
			std::cout << "Usage: ./ircserv <port> <password>\n";
			return (1);
		}
		Server server(argv[1], argv[2]);
		server.start();
	}
	catch (ServerQuitException &err)
	{
		(void)err;
	}
	catch (std::exception &err)
	{
		std::cout << "Error: " << err.what() << "\n";
		return (1);
	}
	return (0);
}
