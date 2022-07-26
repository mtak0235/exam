#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

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

int ft_execute(char **argv, int i, char **env, int temp_fd) {
    argv[i] = 0;
    dup2(temp_fd, STDIN_FILENO);
    close(temp_fd);
    execve(argv[0], argv, env);
    err_print("evec failed!", argv[0]);
    return 1;
}

int main(int argc, char **argv, char **env) {
    argc = 0;
    int fd[2];
    int temp_fd = dup(STDIN_FILENO);
    int i = 0;
    int pid = 0;

    while (argv[i] && argv[i + 1]) {
        argv = &argv[i + 1];
        i = 0;
        while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
            i++;
        if (strcmp(argv[0], "cd") == 0) {
            if (i != 2)
                err_print("bad argument!", NULL);
            else if (chdir(argv[1]) != 0)
                err_print("bad argument!", argv[1]);
        }
        else if (i != 0 && (argv[i] == 0 || strcmp(argv[i], ";") == 0)) {
            pid = fork();
            if (pid == 0) {
                if (ft_execute(argv, i, env, temp_fd))
                    return (1);
            }
            else {
                close(temp_fd);
                while (waitpid(-1, NULL, WUNTRACED) != -1)
                    ;
                temp_fd = dup(STDIN_FILENO);
            }
        }
        else if (i != 0 && strcmp(argv[i], "|") == 0) {
            pipe(fd);
            pid = fork();
            if (pid == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                if (ft_execute(argv, i, env, temp_fd))
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