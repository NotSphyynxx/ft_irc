#include "Server.hpp"


int myport(char *port)
{
	std::string  pt = port;

	if (pt.empty())
		throw std::runtime_error("Port cannot be empty!");
	for (size_t i = 0; i < pt.size(); i++)
	{
		if (!isdigit(pt[i]))
			throw std::runtime_error("Invalid input ! \n");
	}
	long to_nb = atol(port);

	if (to_nb > 65535 || to_nb < 0)
		throw std::runtime_error("Overflow occured ! \n");

	if (to_nb < 1024)
		throw std::runtime_error("that port is reserved ! \n");

   return (static_cast<int> (to_nb));
}

bool mypass(char *pass)
{
   std::string pw = pass;
   if (pw.empty())
		throw std::runtime_error("Empty password ! \n");
	return true;
}

void toUpper(std::string &s)
{
	for (size_t i = 0; i < s.length(); ++i)
	{
		// Cast to unsigned char to handle 8-bit characters safely
		s[i] = (char)std::toupper((unsigned char)s[i]);
	}
}

std::string bot_ascii_trim_line(const std::string& target, const std::string& raw_text)
{
	std::stringstream ss(raw_text);
	std::string line;
	std::string final_syntax = "";

	// This loop safely chops the raw text at every '\n'
	while (std::getline(ss, line))
	{
		// Clean off any lingering \r
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);

		// Skip completely empty lines so the IRC server doesn't disconnect you for spam
		if (line.empty())
			continue;

		// Forge the individual PRIVMSG command and stack it
		final_syntax += bot_CMD_PRIVMSG(target, line);
	}
	return final_syntax;
}


