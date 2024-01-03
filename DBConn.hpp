#ifndef DBCONN_HPP
#define DBCONN_HPP

#pragma once

#include <iostream>
#include <libpq-fe.h>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <time.h>


#define DB_HOST "127.0.0.1"
#define DB_PORT "5432"
#define DB_USER "postgres"
#define DB_PASS "postgres"
#define DB_NAME "irc"


inline std::string fmt_str(const char* format, ...)
{
	std::string	buf;
	va_list		args;
	int			size;
	int			written;

	va_start(args, format);
	size = std::vsnprintf(nullptr, 0, format, args);
	va_end(args);
	if (size <= 0)
		return std::string();
	
	buf.reserve(1);
	buf.resize(static_cast<size_t>(size) + 1);
	va_start(args, format);
	written = std::vsnprintf(&buf[0], buf.size(), format, args);
	va_end(args);
	if (written < 0)
		return std::string();

	buf.resize(static_cast<size_t>(written));
	return (buf);
}


class DBConn
{
	private:
		PGconn	*conn;

		void	setup(std::string const &connect);
		void	finish();

	public:
		int		status;

		DBConn(
			std::string const &hostname,
			std::string const &port,
			std::string const &username,
			std::string const &password,
			std::string const &dbname);
		DBConn(
			const char *hostname,
			const char *port,
			const char *username,
			const char *password,
			const char *dbname);
		~DBConn();

		PGresult *exec(std::string const &cmd);
		PGresult *query(std::string const &cmd);
};

#include <algorithm>

extern DBConn	*db;

inline void	console_log(std::string const &msg, int save=1)
{
	time_t		raw_time;
	struct tm	*time_info;
	char		buffer[80];

	time(&raw_time);
	time_info = localtime(&raw_time);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", time_info);
	std::string now(buffer);
	std::string strip = msg.c_str();
	strip.erase(std::remove_if(strip.begin(), strip.end(),
                    [](char c) { return c == '\r' || c == '\n'; }),
                    strip.end());
	std::cout << "[" << now << "] " << strip << std::endl;
	if (db != nullptr && save)
	{
		PGresult *res = db->exec(fmt_str(
			"INSERT INTO logs (log, date) VALUES ('%s', '%s')", strip.c_str(), now.c_str()));
		PQclear(res);
	}
}

#endif
