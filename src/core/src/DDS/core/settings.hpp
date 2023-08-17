#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <map>
#include <string>

class settings
{
public:
	settings(settings const&) = delete;
	void operator=(settings const&) = delete;
	static settings& get()
	{
		static settings instance;
		return instance;
	}

	void set_loglevel(int);

	std::map<std::string, int> dint;
	std::map<std::string, bool> dbool;
	std::map<std::string, std::string> dstring;
private:
	settings();
};

#endif // !SETTINGS_HPP
