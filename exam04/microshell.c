#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#ifdef TEST_SH
# define TEST		1
#else
# define TEST		0
#endif

int ft_strlen(char *str) {
    int cnt = 0;
    if (!str)
        return cnt;
    while (str[cnt++]);
    return cnt;
}

void ft_err(char *s1, char *s2) {
    write(2, s1, ft_strlen(s1));
    write(2, s2, ft_strlen(s2));
    write(2, "\n", 1);
}

int ft_execute(char **av, int i, int tfd, char **env) {
    av[i] = 0;//없어도 되지 않을까
    dup2(tfd, STDIN_FILENO);
    close(tfd);
    execve(av[0], av, env);
    ft_err("error: cannot execute ", av[0]);
    return 1;
}
int main(int ag, char **av, char **env) {
    ag = 0;
    int i = 0;
    int pid = 0;
    int tfd = dup(STDIN_FILENO);
    int fd[2];

    while(av[i] && av[i + 1]) {
        av = &av[i + 1];
        i = 0;
        while(av[i] && strcmp(av[i], "|") && strcmp(av[i], ";")) {
            // printf("\033[34mfirst while : av[%d] = %s\n\033[37m", i, av[i]);
            i++;
        }
        if (strcmp(av[0], "cd") == 0) {
            if (i != 2)
                ft_err("error: cd: bad arguments", NULL);
            else if(chdir(av[1]) != 0)
                ft_err("error: cd: cannot change directory to ", av[1]);
        }
        else if (i != 0 && (av[i] == 0 || strcmp(av[i],  ";") == 0)) {
            pid = fork();
            if (pid == 0) {
                if (ft_execute(av, i, tfd, env))
                    return 1;;
            }
            close(tfd);
            waitpid(-1, NULL, 0);
            tfd = dup(STDIN_FILENO);
        }
        else if (i != 0 && strcmp(av[i] , "|") == 0) {
            pipe(fd);
            pid = fork();
            if (pid == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                if (ft_execute(av, i, tfd, env))
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