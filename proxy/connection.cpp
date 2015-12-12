#include "connection.hpp"

Connection::Connection(int sock) : len(0), sock(sock) {
	buffer = (char *)calloc(1, BUFLEN);
}

Connection::~Connection() {
	close(sock);
}

int Connection::recv_data() {
	free(buffer);
	buffer = (char * )calloc(1, BUFLEN);
	len = read(sock, buffer, BUFLEN);
	return len;
}

int Connection::send_data(const char * data, int len) {
	int sent = send(sock, data, len, 0);
	std::cout << "Required length to send : " << len << std::endl;
	std::cout << "DATA TO SEND : " << std::endl;
	write(1, data, len);
	std::cout << "Actually sent : " << sent << std::endl;
	return sent;
}

void Connection::close_sock() {
	shutdown(sock, SHUT_RDWR);
	close(sock);
}

char * Connection::get_buffer() {
	return buffer;
}

int Connection::get_sock() const {
	return sock;
}

void Connection::set_sock(int s) {
	sock = s;
}

int Connection::get_length() const {
	return len;
}