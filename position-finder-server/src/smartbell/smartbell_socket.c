#include"smartbell.h"

int smart_bell_socket_connect(char* data){
	int   client_socket;
	struct sockaddr_in   server_addr;
	char   buff[BUFF_SIZE+5];

	client_socket  = socket( PF_INET, SOCK_STREAM, 0);
	if( -1 == client_socket){
		_D("socket 생성 실패");
		return -1;
	}

	memset( &server_addr, 0, sizeof( server_addr));
	server_addr.sin_family     = AF_INET;
	server_addr.sin_port       = htons(33333);
	server_addr.sin_addr.s_addr= inet_addr( "192.168.43.79");

	if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) ){
		_D("접속 실패");
		return -1;
	}
	write( client_socket, data, strlen(data)+1);      // +1: NULL까지 포함해서 전송
	read ( client_socket, buff, BUFF_SIZE);
	_D("%s", buff);
	close( client_socket);

	return 0;
}
