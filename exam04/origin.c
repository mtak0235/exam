#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

#ifdef TEST_SH
# define TEST		1
#else
# define TEST		0
#endif

int ft_strlen(char *s) {
    int i = 0;

    if (s == NULL)
        return 0;
    while (s[i])
        i++;
    return i;
}

void err_print(char *s1, char *s2) {
    write(2, s1, ft_strlen(s1));
    write(2, s2, ft_strlen(s2));
    write(2, "\n", 1);
}

int ft_execute(char **av, int i, char **env, int temp_fd) {
    av[i] = 0;
    dup2(temp_fd, STDIN_FILENO);
    close(temp_fd);
    execve(av[0], av, env);
    err_print("evec failed!", av[0]);
    return 1;
}

int main(int argc, char **av, char **env) {
    argc = 0;
    int fd[2];
    int temp_fd = dup(STDIN_FILENO);
    int i = 0;
    int pid = 0;

    while (av[i] && av[i + 1]) {
        printf("\033[31m1st loop\n i == %d\n av[i] = %s, av[i + 1]= %s\033[37m\n",i, av[i], av[i + 1]);
        av = &av[i + 1];
        i = 0;
        while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
            i++;
        printf("\033[32m2nd loop\n i==%d\n av[i] = %s\n\033[37m", i, av[i]);
        if (strcmp(av[0], "cd") == 0) {
            if (i != 2)
                err_print("bad argument!", NULL);
            else if (chdir(av[1]) != 0)
                err_print("bad argument!", av[1]);
        }
        else if (i != 0 && (av[i] == 0 || strcmp(av[i], ";") == 0)) {
            printf("\033[33m2nd elif\n av[%d] = %s\033[37m\n", i, av[i]);
            pid = fork();
            if (pid == 0) {
                if (ft_execute(av, i, env, temp_fd))
                    return (1);
            }
            else {
                close(temp_fd);
                while (waitpid(-1, NULL, WUNTRACED) != -1)
                    ;
                temp_fd = dup(STDIN_FILENO);
            }
        }
        else if (i != 0 && strcmp(av[i], "|") == 0) {
            printf("\033[33m3rd elif\n av[%d] = %s\033[37m\n", i, av[i]);
            pipe(fd);
            pid = fork();
            if (pid == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                if (ft_execute(av, i, env, temp_fd))
                    return (1);
            }
            else {
                close(temp_fd);
                close(fd[1]);
                temp_fd = fd[0];
            }
        }
    }
    close(temp_fd);
    return 0;
}