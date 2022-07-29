#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
int ft_strlen(char *s) {
    int cnt = 0;
    if (!s)//실수 지점
        return cnt;
    while (s[cnt++]);
    return cnt;
}

void ft_error(char *msg, char *param) {
    write(2, msg, ft_strlen(msg));
    write(2, param, ft_strlen(param));
    write(2, "\n", 1);
}

int ft_execute(char **av, char **env, int tfd, int i) {
    av[i] = 0;
    dup2(tfd, STDIN_FILENO);
    close(tfd);
    execve(av[0], av, env);
    ft_error("error: cannot execute ", av[0]);
    return 1;
}
int main(int ag, char **av, char **env){
    int pid = 0;
    int i = 0;
    int tfd = dup(STDIN_FILENO);
    int fd[2];
    
    while(av[i] && av[i + 1]) {
        av = &av[i + 1];
        i = 0; // 실수 지점
        while (av[i] && strcmp(av[i],"|") && strcmp(av[i],";"))
            i++;
        if (!strcmp(av[0], "cd")) {
            if (i != 2)
                ft_error("error: cd: bad arguments", NULL);
            if (chdir(av[1]))
                ft_error("error: cd: cannot change directory to ", av[1]);
        } else if (i != 0 && (av[i]== 0 ||strcmp(av[i], ";") == 0))//null먼저 검사해야됨.
        {
            pid = fork();
            if (pid == 0) {
                if (ft_execute(av, env, tfd, i))
                    return 1;
            }
            close(tfd);
            waitpid(-1, NULL, 0);
            tfd = dup(STDIN_FILENO);
        } else if (i != 0 && strcmp(av[i], "|") == 0)
        {
            pipe(fd);
            pid = fork();
            if (pid == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                if (ft_execute(av, env, tfd, i))
                    return 1;
            }
            close(tfd);
            close(fd[1]);
            tfd = fd[0];
        }
    }
    close(tfd);
    return 0;
}