#include "Server.hpp"

Server::Server(std::string const &port, std::string const &password)
{
	this->host = HOST;
	this->port = port;
	this->password = password;
	this->running = 1;
	this->sock = this->create_socket();
	this->handler = new CommandHandler((this));

	PGresult	*res = db->query("SELECT * FROM channels;");
	int	rows = PQntuples(res);
	std::string	cname;
	std::string	cpass;
	for (int row = 0; row < rows; ++row)
	{
		cname = PQgetvalue(res, row, 0);
		cpass = PQgetvalue(res, row, 1);
		Channel	*channel = new Channel(cname, cpass);
		this->channels.push_back(channel);
		console_log(fmt_str("Loaded Channel %s:%s", cname.c_str(), cpass.c_str()));
	}
	PQclear(res);
}

Server::~Server()
{
	std::vector<int> fds;

	for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
		fds.push_back(it->second->getFd());
	for (int fd = 0; fd != (int)fds.size(); ++fd)
	{
		this->clients[fds[fd]]->msgReply("Shutting down the server\n");
		this->handle_disconnection(fds[fd]);
	}
	for (int channel = 0; channel != (int)this->channels.size(); ++channel)
		delete this->channels.at(channel);
	delete this->handler;
	close(this->sock);
	console_log("Main Socket Closed");
}

void	handle_sigint(int sig)
{
	(void)sig;
	throw ServerQuitException();
}

void handle_sigpipe(int sig)
{
	(void)sig;
}

void	Server::start()
{
	pollfd	server_fd = {this->sock, POLLIN, 0};
	poll_fds.push_back(server_fd);

	if (!MAC_OS)
		signal(SIGINT, handle_sigint);
	signal(SIGPIPE, handle_sigpipe);
	console_log(fmt_str("Server waiting for connections on %s:%s", this->host.c_str(), this->port.c_str()));

	while (this->running)
	{
		if (poll(poll_fds.begin().base(), poll_fds.size(), -1) < 0)
			throw std::runtime_error("Error while polling");
		for (std::vector<pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
		{
			if (it->revents == 0)
				continue ;
			if ((it->revents & POLLIN) == POLLIN)
			{
				if (it->fd == this->sock)
				{
					this->handle_connection();
					break ;
				}
				if (this->handle_message(it->fd))
					break ;
			}
			if ((it->revents & POLLHUP) == POLLHUP)
			{
				this->handle_disconnection(it->fd);
				break ;
			}
		}
	}
}

int	Server::create_socket()
{
	int	sockfd;
	int	int_port;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		throw std::runtime_error("Error while opening socket");
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("Error while setting socket to NON-BLOCKING");
	struct sockaddr_in addr = {};
	bzero((char *)&addr, sizeof(addr));
	std::istringstream(this->port) >> int_port;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(int_port);
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Error while binding socket");
	if (listen(sockfd, MAX_CONNECTIONS) < 0)
		throw std::runtime_error("Error while listening in socket");
	return (sockfd);
}

void	Server::handle_connection()
{
	int			fd;
	sockaddr_in	addr = {};
	socklen_t 	size;
	std::string	ip_addr;

	size = sizeof(addr);
	fd = accept(this->sock, (sockaddr *)&addr, &size);
	if (fd < 0)
		throw std::runtime_error("Error while accepting new client");
	pollfd	poll_fd = {fd, POLLIN, 0};
	this->poll_fds.push_back(poll_fd);
	ip_addr = inet_ntoa(addr.sin_addr);
	if (getsockname(fd, (struct sockaddr *)&addr, &size) != 0)
		throw std::runtime_error("Error while gathering client informations");
	Client *new_client = new Client(fd, ip_addr, ntohs(addr.sin_port));
	this->clients.insert(std::make_pair(fd, new_client));
	console_log(new_client->log("has connected"));
}

std::string	Server::recive(int fd)
{
	std::string	msg;
	char		buffer[100];

	bzero(buffer, 100);
	while (!std::strstr(buffer, "\n"))
	{
		bzero(buffer, 100);
		if (recv(fd, buffer, 100, 0) < 0)
		{
			if (errno != EWOULDBLOCK)
				throw std::runtime_error("Error while reciving from client");
		}
		if (!buffer[0])
			return ("");
		msg.append(buffer);
	}
	return (msg);
}

int	Server::handle_message(int fd)
{
	std::string msg = this->recive(fd);
	#if DEBUG
		console_log(msg);
	#endif
	if (msg[0] == 0)
	{
		this->handle_disconnection(fd);
		return (1);
	}
	Client	*client = this->clients.at(fd);
	if (this->handler->handle_command(client, msg))
	{
		this->handle_disconnection(client->getFd());
		return (1);
	}
	return (0);
}

void	Server::handle_disconnection(int fd)
{
	try
	{
		Client	*client = this->clients.at(fd);

		client->leave();
		this->clients.erase(fd);
		for (std::vector<pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it)
		{
			if (it->fd != fd)
				continue ;
			this->poll_fds.erase(it);
			close(fd);
			break ;
		}
		console_log(client->log("has disconnected"));
		delete client;
	}
	catch (std::out_of_range const &err) {}
}

Client	*Server::getClient(std::string const &name)
{
	std::map<int, Client *>::iterator it;

	for (it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		if (!name.compare(it->second->getNickname()))
			return (it->second);
	}
	return (nullptr);
}

Channel	*Server::getChannel(std::string const &name)
{
	std::vector<Channel *>::iterator it;

	for (it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		if (!name.compare(it.operator*()->getName()))
			return (it.operator*());
	}
	return (nullptr);
}

Channel	*Server::createChannel(std::string const &name, std::string const &password)
{
	Channel	*channel = new Channel(name, password);
	this->channels.push_back(channel);

	PGresult	*res;
	if (password.empty())
		res = db->exec(fmt_str(
			"INSERT INTO channels (name) VALUES ('%s')", name.c_str()));
	else
		res = db->exec(fmt_str(
			"INSERT INTO channels (name, password) VALUES ('%s', '%s')",
			name.c_str(), password.c_str()));
	PQclear(res);

	return (channel);
}
