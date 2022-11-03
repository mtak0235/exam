#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h> //gethostname()
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h> //gethostbyname()
#include <cstring> //memcpy(), strcpy()
#include <fstream>
#include <fcntl.h>
#include <map>
#include <vector>

#define BUFSIZE 50
#define PORT_NUM 8000

int	getHostAddr()
{
	char hostName[256], hostIP[16];
	struct hostent *host;
	struct in_addr ip;

	gethostname(hostName, (int)sizeof(hostName));
	host = gethostbyname(hostName);
	for (int i = 0; host->h_addr_list[i]; i++)
		memcpy(&ip, host->h_addr_list[i], 4);;
	strcpy(hostName, inet_ntoa(ip));
	std::cout << "build server : http://" << hostName << ":" << PORT_NUM << std::endl;
	return 0;
}

void error_handler(std::string msg)
{
	std::cout << msg << std::endl;
	exit(1);
}

void log_handler(std::string log)
{
	std::cout << log << std::endl;
}

void getRequestPath(std::string &path, char *request)
{
	if (!strncmp(request, "GET ", 4))
		path = strtok(request + 4, " ");
	else if (!strncmp(request, "POST ", 5))
		path = strtok(request + 5, " ");
	else if (!strncmp(request, "DELETE ", 6))
		path = strtok(request + 6, " ");
	log_handler("request path : " + path);
}

std::string openFile(std::string file_name)
{
	std::string one_line;
	std::string ret;
	std::ifstream fin(file_name);

	if (!fin.is_open())
		return NULL;
	while (std::getline(fin, one_line))
		ret = ret + one_line + "\n";
	fin.close();
	return ret;
}

std::string getResponseHeader()
{
	return openFile("header");
}

std::string getResponseBody(std::string path)//여기서 url 파싱 해야됨. ex. /directory/*.bla, /save //?니오면 유효하지않은 놈으로 처리.
{
	std::string ret;

	if (!strcmp(path.c_str(), "/"))
		ret = openFile("hello.html");
	else if(!strcmp(path.c_str(), "/world"))
		ret = openFile("world.html");
	return ret;
}

int main(int argc, char **argv){
	fd_set reads; // 감시할 소켓 목록( 여기서는 소켓의 입력스트림 감시 용도 ) 
	fd_set temps; // reads 변수의 복사본으로 사용도리 변수
	struct timeval timeout; // 타임 변수
	int fd_max; // 최대 검사할 파일디스립터의 수 
	int result; // select() 함수의 리턴값 저장 
	char message[BUFSIZE]; 
	int serv_sock; 
	struct sockaddr_in serv_addr; 

	if (getHostAddr())
	{
		printf("Failed occupying IP address");
		return -1;
	}
	//서버 소켓 만듦.
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (serv_sock == -1)
		error_handler("socket error");
	//서버 소켓 설정
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(PORT_NUM);
	//서버 소켓 바인딩
	if(bind(serv_sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1 ) exit(1);
	log_handler("binding successful");
	//요청 대기 시작
	if(listen(serv_sock, 5) == -1) exit(1); 
	log_handler("listening successful");
	FD_ZERO(&reads); // reads 를 0 으로 초기화 
	FD_SET(serv_sock, &reads); // 서버용소켓을 감시대상으로 등록 
	fd_max = serv_sock; // 파일디스립터 번호는 2부터 차례대로 등록되기 때문에 // 마지막에 생성한 소켓의 번호는 총 소켓의 갯수가 된다. // (0,1 은 시스템의 입력/출력 용도로 사용됨) 
	while(true)
	{ 
		int clnt_sock; 
		struct sockaddr_in clnt_addr; 
		socklen_t clnt_addr_size; 
		int read_len; 
		int fd; 
		
		temps=reads; // fd_set 변수 복사 
		timeout.tv_sec = 5; // 5초 
		timeout.tv_usec = 0; // 검사 갯수는 0 ~ n-1 로 계산되니 때문에 fd_max+1 삽입 // 소켓의 입력스트림 검사 // 나머지는 검사 안함 // 5초 대기 
		result = select(fd_max+1, &temps, 0,0,&timeout); 
		if(result == -1) break; 
		if(result == 0) { log_handler("running servser.."); continue; } 
		for(fd = 0 ; fd < fd_max+1 ; fd++)
		{ // 검사 갯수만큼 루프문 돌면서 검사 
			if(FD_ISSET(fd, &temps) == 0)
				continue; // FD_ISSET() 은 리스트에 1로 되어 있는 소켓을 찾는다. // 즉, 클라이언트로부터 데이터가 날라와서 입력스트림에 뭔가 있는가? 
			if(fd != serv_sock)
			{ // 클라이언트 소켓이면 
				read_len = recv(fd,message,sizeof(message) - 1, 0); 
				if(read_len > 0) { 
					message[read_len] = '\0';
					std::string path;
					getRequestPath(path, message);
					std::string response_header = getResponseHeader();
					std::string response_body = getResponseBody(path);
					std::string response = response_header + "\n" + response_body;
					std::cout << fd << std::endl;
					std::cout << "\033[32m";
					log_handler(response);
					std::cout << "\033[37m";

					send(fd, response.c_str(), (int)strlen(response.c_str()), 0);
					printf("Disconnect client : %d \n",fd); 
					FD_CLR(fd,&reads); // 감시목록에서 제외 
					close(fd); // 통신 종료 
					if(fd_max == fd)
						fd_max--; // 갯수에서도 제외 
				} 
			}
			else
			{ // 서버 소켓이면, 연결요청이 들어온것이므로 연결작업시작 
				clnt_addr_size = sizeof(clnt_addr); 
				clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size); 
				if(clnt_sock == -1)
					continue;
				FD_SET(clnt_sock, &reads);  // 새로 생성한 클라이언트용 소켓을 감시대상에 추가 
				if(fd_max<clnt_sock)
					fd_max = clnt_sock; 
				printf("Connect New Client : %d \n",clnt_sock); 
			} 
		} 
	}
	close(serv_sock);
	return 0;
}
