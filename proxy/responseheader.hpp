#ifndef RESPONSE_H
#define RESPONSE_H

#include <string>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <iostream>

class ResponseHeader {
 public:
	ResponseHeader(char * data);
	~ResponseHeader();
	int get_code() const;
	int get_length() const;
	int get_header_len() const;

	static const int OK_CODE = 200;
 private:
 	int code;
 	int header_len;
 	int content_len;
 	std::string protocol;

 	void parse_header(char *header);
};

#endif