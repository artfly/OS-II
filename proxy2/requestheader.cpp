#include "requestheader.hpp"

const std::string RequestHeader::GET_METHOD = "GET";
const std::string RequestHeader::HEAD_METHOD = "HEAD";
const std::string RequestHeader::SUPPORTED_VERSION = "HTTP/1.0";
const std::string RequestHeader::SUPPORTED_PROTOCOL = "http";

const std::string RequestHeader::MESSAGE_502 = "HTTP/1.0 502 Bad Gateway\r\n\
\r\n<html><head><title>502 Bad Gateway</title></head> \
<body><h2>502 Bad Gateway</h2><h3>Host not found or connection failed.</h3></body></html>\r\n";
const std::string RequestHeader::MESSAGE_505 = "HTTP/1.0 505 HTTP Version not supported\r\n\
\r\n<html><head><title>505 HTTP Version not supported</title></head> \
<body><h2>505 HTTP Version not supported</h2><h3>Proxy server does not support the HTTP protocol version used in the request.</h3></body></html>\r\n";
const std::string RequestHeader::MESSAGE_405 = "HTTP/1.0 405 Method Not Allowed\r\n\
\r\n<html><head><title>405 Method Not Allowed</title></head> \
<body><h2>405 Method Not Allowed</h2><h3>Proxy server does not support the HTTP method used in the request.</h3></body></html>\r\n";
const std::string RequestHeader::MESSAGE_414 = "HTTP/1.0 414 Request-URL Too Long\r\n\
\r\n<html><head><title>414 Request-URL Too Long</title></head> \
<body><h2>414 Request-URL Too Long</h2><h3>The URL provided was too long for the server to process.</h3></body></html>\r\n";
const std::string RequestHeader::MESSAGE_500 = "HTTP/1.0 500 Internal Server Error\r\n\
\r\n<html><head><title>500 Internal Server Error</title></head> \
<body><h2>500 Internal Server Error</h2></body></html>\r\n";


RequestHeader::RequestHeader(std::string data) : data(data), error(false) {
	std::stringstream ss(data);
	ss >> method >> url >> version;
	protocol = url.substr(0, url.find("://"));
	url = url.substr(url.find("://") + 3, url.length());
	parse_url();
}

RequestHeader::~RequestHeader() {}

void RequestHeader::parse_url() {
	std::size_t colon_pos = url.find_last_of(":");
	std::string maybe_port = "-1";
	if (colon_pos != std::string::npos) {
		maybe_port = url.substr(colon_pos + 1, url.length());
		for (size_t i = 0; i < maybe_port.length(); i++) {
			if (i > 3 || !std::isdigit(maybe_port[i])) {
				maybe_port = "-1";
				break;
			}
		}
	}
	if (maybe_port != "-1") {			
		port = atoi(maybe_port.c_str());
		host = url.substr(0, colon_pos - 1);
	}
	else {
		host = url.substr(0, url.find("/"));
		port = 80;
	}
}

bool RequestHeader::check_header() {
	if(data.find("\r\n") == std::string::npos) {
		error_msg = MESSAGE_414;
		error = true;
		return false;
	}
	if (method.compare(HEAD_METHOD) && method.compare(GET_METHOD)) {
		error_msg = MESSAGE_405;
		error = true;
		return false;
	}
	// if (version.compare(SUPPORTED_VERSION)) {
	// 	error_msg = MESSAGE_505;
	// 	error = true;
	// 	return false;
	// }
	if (protocol.compare(SUPPORTED_PROTOCOL)) {
		error_msg = MESSAGE_500;
		error = true;
		return false;
	}
	return true;
}

std::string RequestHeader::get_url() const {
	return url;
}

std::string RequestHeader::get_method() const {
	return method;
}

std::string RequestHeader::get_version() const {
	return version;
}

std::string RequestHeader::get_protocol() const {
	return protocol;
}

std::string RequestHeader::get_host() const {
	return host;
}

int RequestHeader::get_port() const {
	return port;
}

bool RequestHeader::check_error() {
	return error;
}

std::string RequestHeader::get_error_msg() const {
	return error_msg;
}

std::string RequestHeader::get_data() const {
	return data;
}