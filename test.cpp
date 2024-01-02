#include <cstdio>
#include <string>
#include <iostream>
#include <cstdarg>

std::string	formatString(const char* format, ...)
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

int main() {
	std::string result = formatString("This is a formatted string with %d and %s", 42, "hello");
	std::cout << result << std::endl;

	return 0;
}
