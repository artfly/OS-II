#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <sstream>
#include <cstddef>
#include <stdlib.h>
#include <cctype>

class RequestHeader {
 public:
	RequestHeader(std::string data);
	~RequestHeader();

	bool check_header();
	bool check_error();
	std::string get_url() const;
	std::string get_error_msg() const;
	std::string get_version() const;
	std::string get_protocol() const;
	std::string get_method() const;
	std::string get_host() const;
	std::string get_data() const;
	int get_port() const;
 private:
 	static const std::string GET_METHOD;
 	static const std::string HEAD_METHOD;
 	static const std::string SUPPORTED_VERSION;
 	static const std::string SUPPORTED_PROTOCOL;

 	static const std::string MESSAGE_502;
 	static const std::string MESSAGE_505;
 	static const std::string MESSAGE_414;
 	static const std::string MESSAGE_500;
 	static const std::string MESSAGE_405;

 	void parse_url();

 	std::string method;
 	std::string url;
 	std::string version;
 	std::string protocol;
 	std::string data;

 	int port;
 	std::string host;

 	std::string error_msg;
 	bool error;
};

#endif