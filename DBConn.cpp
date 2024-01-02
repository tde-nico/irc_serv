#include "DBConn.hpp"

DBConn::DBConn(
			std::string const &hostname,
			std::string const &port,
			std::string const &user,
			std::string const &password,
			std::string const &dbname)
{
	std::string	buf;

	buf = fmt_str(
		"host=%s port=%s user=%s password=%s dbname=%s",
		hostname,
		port,
		user,
		password,
		dbname
	);
	this->conn = PQconnectdb(buf.c_str());
	if (PQstatus(this->conn) != CONNECTION_OK)
	{
		std::cerr << fmt_str("Error DBConn: %s\n", PQerrorMessage(this->conn));
		this->finish();
	}
}

DBConn::~DBConn()
{
	this->finish();
}

void	DBConn::finish()
{
	PQfinish(this->conn);
	this->conn = nullptr;
}

PGresult	*DBConn::exec(std::string const &cmd)
{
	PGresult	*res;

	if (this->conn == nullptr)
	{
		std::cerr << "No DB Connection\n";
		return (nullptr);
	}
	res = PQexec(this->conn, cmd.c_str());
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		std::cerr << fmt_str(
			"Error DBConn: cmd:%s status:%s err:%s\n",
			cmd,
			PQresStatus(PQresultStatus(res)),
			PQresultErrorMessage(res)
		);
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
		std::cerr << "No DB Connection\n";
		return (nullptr);
	}
	res = PQexec(this->conn, cmd.c_str());
	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		std::cerr << fmt_str(
			"Error DBConn: cmd:%s err:%s\n",
			cmd,
			PQresultErrorMessage(res)
		);
		PQclear(res);
		this->finish();
		return (nullptr);
	}
	return (res);
}
