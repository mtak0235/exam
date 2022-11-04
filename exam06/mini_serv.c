#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct		s_client 
{
	int				fd;
    int             id;
	struct s_client	*next;
}	t_client;

t_client *g_clients = NULL;

int serv_sock, g_id = 0;
fd_set reads; //감시할 소켓 목록
fd_set cpy_read, cpy_write;
char msg[42];
char str[42*4096], tmp[42*4096], buf[42*4096 + 42];

void	fatal() 
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(serv_sock);
	exit(1);
}

int get_id(int fd)
{
    t_client *temp = g_clients;

    while (temp)
    {
        if (temp->fd == fd)
            return (temp->id);
        temp = temp->next;
    }
    return (-1);
}

int		get_max_fd() 
{
	int	max = serv_sock;
    t_client *temp = g_clients;

    while (temp)
    {
        if (temp->fd > max)
            max = temp->fd;
        temp = temp->next;
    }
    return (max);
}

void	send_all(int fd, char *str_req)
{
    t_client *temp = g_clients;

    while (temp)
    {
        if (temp->fd != fd && FD_ISSET(temp->fd, &cpy_write))
        {
            if (send(temp->fd, str_req, strlen(str_req), 0) < 0)
                fatal();
        }
        temp = temp->next;
    }
}

int		add_client_to_list(int fd)
{
    t_client *temp = g_clients;
    t_client *new;

    if (!(new = calloc(1, sizeof(t_client))))
        fatal();
    new->id = g_id++;
    new->fd = fd;
    new->next = NULL;
    if (!g_clients)
    {
        g_clients = new;
    }
    else
    {
        while (temp->next)
            temp = temp->next;
        temp->next = new;
    }
    return (new->id);
}

void add_client()
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int client_fd;

    if ((client_fd = agcept(serv_sock, (struct sockaddr *)&clientaddr, &len)) < 0)
        fatal();
    sprintf(msg, "server: client %d just arrived\n", add_client_to_list(client_fd));
    send_all(client_fd, msg);
    FD_SET(client_fd, &reads);
}

int rm_client(int fd)
{
    t_client *temp = g_clients;
    t_client *del;
    int id = get_id(fd);

    if (temp && temp->fd == fd)
    {
        g_clients = temp->next;
        free(temp);
    }
    else
    {
        while(temp && temp->next && temp->next->fd != fd)
            temp = temp->next;
        del = temp->next;
        temp->next = temp->next->next;
        free(del);
    }
    return (id);
}

void ex_msg(int fd)
{
    int i = 0;
    int j = 0;

    while (str[i])
    {
        tmp[j] = str[i];
        j++;
        if (str[i] == '\n')
        {
            sprintf(buf, "client %d: %s", get_id(fd), tmp);
            send_all(fd, buf);
            j = 0;
            bzero(&tmp, strlen(tmp));
            bzero(&buf, strlen(buf));
        }
        i++;
    }
    bzero(&str, strlen(str));
}

int main(int ag, char **av)
{
    if (ag != 2) {
        write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
        exit(1);
    }
    struct sockaddr_in serv_addr;
    //서버 소켓 만듦
    if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        fatal();
    //서버 소켓 설정
    uint16_t port = atoi(av[1]);
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
	serv_addr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	serv_addr.sin_port = htons(atoi(av[1]));
    //서버 소켓 바인딩
    if (bind(serv_sock, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        fatal();
    //요청 대기 시작
    if (listen(serv_sock, 0) == -1)
        fatal();
    
    FD_ZERO(&reads); // reads를 0으로 초기화
    FD_SET(serv_sock, &reads);//서버용 소켓을 감시 대상으로 등록

    bzero(&tmp, sizeof(tmp));
    bzero(&buf, sizeof(buf));
    bzero(&str, sizeof(str));

    while(true)
    {
        cpy_write = cpy_read = reads;
        if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) == -1) //실패시 재시도
            continue;
        for (int fd = 0; fd <= get_max_fd(); fd++)
        {
            if (FD_ISSET(fd, &cpy_read))
            {
                if (fd == serv_sock)
                {
                    bzero(&msg, sizeof(msg));
                    add_client();
                    break;
                }
                else
                {
			int ret_recv = 1000;
			while (ret_recv == 1000 || str[strlen(str) - 1] != '\n')
			{
				ret_recv = recv(fd, str + strlen(str), 1000, 0);
				if (ret_recv <= 0)
					break ;
			}
                    if (ret_recv <= 0)
                    {
                        bzero(&msg, sizeof(msg));
                        sprintf(msg, "server: client %d just left\n", rm_client(fd));
                        send_all(fd, msg);
                        FD_CLR(fd, &reads);
                        close(fd);
                        break;
                    }
                    else
                    {
                        ex_msg(fd);
                    }
                }
            }
            
        }
        
    }
    return (0);
}
