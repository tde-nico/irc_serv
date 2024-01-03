#include "DBConn.hpp"

DBConn::DBConn(std::string const &hostname,
	std::string const &port, std::string const &user,
	std::string const &password, std::string const &dbname)
{
	this->setup(fmt_str(
		"host=%s port=%s user=%s password=%s dbname=%s",
		hostname.c_str(), port.c_str(), user.c_str(), password.c_str(), dbname.c_str()
	));
}

DBConn::DBConn(const char *hostname,
	const char *port, const char *user,
	const char *password, const char *dbname)
{
	this->setup(fmt_str(
		"host=%s port=%s user=%s password=%s dbname=%s",
		hostname, port, user, password, dbname
	));
}

void	DBConn::setup(std::string const &connect)
{
	this->conn = PQconnectdb(connect.c_str());
	if (PQstatus(this->conn) != CONNECTION_OK)
	{
		console_log(fmt_str("Error DBConn: %s\n", PQerrorMessage(this->conn)), 0);
		this->finish();
	}
	this->status = 0;
	console_log("DB Connected");
}

DBConn::~DBConn()
{
	this->finish();
}

void	DBConn::finish()
{
	if (this->conn == nullptr)
		return ;
	console_log("DB Disconnected");
	PQfinish(this->conn);
	this->conn = nullptr;
	this->status = 1;
}

PGresult	*DBConn::exec(std::string const &cmd)
{
	PGresult	*res;

	if (this->conn == nullptr)
	{
		console_log("No DB Connection\n", 0);
		return (nullptr);
	}
	res = PQexec(this->conn, cmd.c_str());
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		console_log(fmt_str(
			"Error DBConn: cmd:%s status:%s err:%s\n",
			cmd.c_str(),
			PQresStatus(PQresultStatus(res)),
			PQresultErrorMessage(res)
		), 0);
		PQclear(res);
		this->finish();
		return (nullptr);
	}
	return (res);
}

PGresult	*DBConn::query(std::string const &cmd)
{
	PGresult	*res;

	if (this->conn == nullptr)
	{
		console_log("No DB Connection\n", 0);
		return (nullptr);
	}
	res = PQexec(this->conn, cmd.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		console_log(fmt_str(
			"Error DBConn: cmd:%s err:%s\n",
			cmd.c_str(),
			PQresultErrorMessage(res)
		), 0);
		PQclear(res);
		this->finish();
		return (nullptr);
	}
	return (res);
}
