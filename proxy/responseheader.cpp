#include "responseheader.hpp"

ResponseHeader::ResponseHeader(char * data) : header_len(-1), content_len(-1) {
	std::stringstream ss;
	ss << data;
	ss >> protocol >> code;
	parse_header(data);
}

ResponseHeader::~ResponseHeader() {}

int ResponseHeader::get_code() const {
	return code;
}

void ResponseHeader::parse_header(char *header) {
	char * end_str = strstr(header, "\r\n\r\n");
	if(end_str == NULL) {
		return;
	}
	end_str += 4;
	header_len = end_str - header;
	char *len_str = strstr(header, "Content-Length: ");
	if(len_str != NULL) {
		len_str += 16;
		content_len = atoi(len_str);
	}
}

int ResponseHeader::get_length() const {
	std::cout << "DEBUG : content_len = " << content_len << " header_len = " << header_len << std::endl;
	return content_len;
}

int ResponseHeader::get_header_len() const {
	return header_len;
}