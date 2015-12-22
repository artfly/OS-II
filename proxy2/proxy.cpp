#include "proxy.hpp"

pthread_mutex_t Proxy::proxy_mutex;
pthread_cond_t Proxy::proxy_cond;
int Proxy::threads_num;

Proxy::Proxy(int port) : proxy_socket(-1), poll_size(0) {
	proxy_socket = init_socket(port);
	add_proxy();
	threads_num = 0;
	pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&proxy_mutex, &attr);
    pthread_cond_init(&proxy_cond, NULL);
}

Proxy::~Proxy() {
	close(proxy_socket);
	Cache::reset_instance();
}


int Proxy::init_socket(int port) {
	int s = 0;
	int enable = 1;
	struct sockaddr_in addr;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "error : could not create socket" << std::endl;
		exit(1);	
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		close(s);
		std::cerr << "error : could not set proxy socket options" << std::endl;
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if ((bind( s, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
		close(s);
		std::cerr << "error : could not bind proxy socket" << std::endl;
		exit(1);
	}
	if ((listen(s, BACKLOG)) < 0) {
		close(s);
		std::cerr << "error : could not init listen" << std::endl;
		exit(1);
	}
	return s;
}

void Proxy::add_proxy() {
	memset(poll_list, 0 , sizeof(poll_list));
	poll_list[0].fd = proxy_socket;
	poll_list[0].events = POLLIN;
	poll_size = 1;
}



void Proxy::run() {
	std::cout << "Proxy is up and running" << std::endl;
	while(1) {
		if (poll(poll_list, poll_size, -1) < 0) {
			std::cerr << "eror : poll error" << std::endl;
			continue;
		}

		if (poll_list[0].revents && POLLIN) {
			add_client();
		}
	}
}

void Proxy::add_client() {
	struct sockaddr_in client_addr;
	socklen_t addr_size;
	int client_sock;
	if ((client_sock = accept(proxy_socket, (struct sockaddr*)&client_addr, &addr_size)) < 0) {
		std::cerr << "error : could not add client" << std::endl;
		return;
	}
	// std::cout << "connection from client" << client_sock << std::endl;
	Client * client = new Client(client_sock);

	pthread_t thread;
    int code;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_mutex_lock(&proxy_mutex);
	while (threads_num == MAX_THREADS_NUM)
		pthread_cond_wait(&proxy_cond, &proxy_mutex);
    code = pthread_create(&thread, &attr, &Proxy::client_body, (void *)client);
    threads_num++;
    // std::cout << "DEBUG : threads_num = " << threads_num << std::endl;
    pthread_mutex_unlock(&proxy_mutex);
}

void * Proxy::client_body(void * param) {
	Client * client = (Client *)param;
	client->run();
	delete client;
	// std::cout << "END OF THREAD" << std::endl;
	//cond signal
	pthread_mutex_lock(&proxy_mutex);
	threads_num--;
	// std::cout << "DEBUG : threads_num = " << threads_num << std::endl;
	pthread_cond_signal(&proxy_cond);
	pthread_mutex_unlock(&proxy_mutex);
	return NULL;
}