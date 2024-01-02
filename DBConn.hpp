#ifndef DBCONN_HPP
#define DBCONN_HPP

#pragma once

#include <iostream>
#include <libpq-fe.h>
#include <cstdio>
#include <cstring>
#include <cstdarg>

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
		void	finish();

	public:
		DBConn(
			std::string const &hostname,
			std::string const &port,
			std::string const &username,
			std::string const &password,
			std::string const &dbname);
		~DBConn();

		PGresult *exec(std::string const &cmd);
		PGresult *query(std::string const &cmd);
};

#endif
