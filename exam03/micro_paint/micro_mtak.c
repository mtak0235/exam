#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
typedef struct s_paper {
    int width;
    int height;
    char c;
} t_paper;

typedef struct s_rectangle {
    char r;
    float lx;
    float ly;
    float w;
    float h;
    char b;
} t_rectangle;

int ft_strlen(char *s) {
    int cnt = 0;
    while (s[cnt++]);
    return cnt;
}
int ft_err(char *msg) {
    if (msg)
        write(1, msg, ft_strlen(msg));
    return 1;    
}

int clear(FILE *fd, char *paper, char *msg) {
    if (fd)
        fclose(fd);
    if (msg)
        ft_err(msg);
    if (paper)
        free(paper);
    return 1;
}
int is_valid_size(int n) {
    if (!(0 < n && n <= 300))
        return 0;
    return 1;
}
char *get_paper(FILE *fd, t_paper *pi) {
    char *paper;

    if (fscanf(fd, "%d %d %c\n", &pi->width, &pi->height, &pi->c) != 3)
        return NULL;
    if (!(is_valid_size(pi->height) && is_valid_size(pi->width)))
        return NULL;
    if (!(paper = (char *)malloc(sizeof(char) * (pi->width * pi-> height))))
        return NULL;
    for (int i = 0; i < pi->height * pi->width; i++)
        paper[i] = pi->c;
    return paper;
}

int is_printable(float x, float y, t_rectangle *rec) {
    if (((x < rec->lx || rec->lx + rec->w < x))||( y < rec->ly || rec->ly + rec->h < y))
        return 0;
    if (rec->r == 'R')
        return 1;
    if ((x - rec->lx < 1. || rec->lx + rec->w - x <1.) || (y - rec->ly < 1. || rec->ly + rec->h - y <1.))
        return 1;
    return 0;
}

void draw_rectangle(char *paper, t_rectangle *rec, t_paper *pi) {
    for (int y = 0; y < pi->height; y++) {
        for (int x = 0; x < pi->width; x++) {
            if (is_printable((float)x, (float)y, rec))
                paper[y * pi->width + x] = rec->b;
        }
    }
}

int draw_rectangles(char *paper, t_paper *pi, FILE *fd) {
    int ret;
    t_rectangle rec;
    while ((ret = fscanf(fd, "%c %f %f %f %f %c\n", &rec.r, &rec.lx, &rec.ly, &rec.w, &rec.h, &rec.b)) == 6) {
        if (!(rec.w > 0 && rec.h > 0) || !(rec.r == 'r' || rec.r == 'R'))
            return 0;
        draw_rectangle(paper, &rec, pi);
    }
    return ret == -1? 1:0;
}

void print_paper(char *paper, t_paper *pi) {
    for (int i = 0; i < pi->height; i++)
    {
        write(1, paper + (i * pi->width), pi->width);
        write(1, "\n", 1);
    }
}
int main(int ag, char **av) {
    FILE *fd;
    char *paper;
    t_paper pi;

    if (ag != 2) 
        return ft_err("Error: argument\n");
    if (!(fd = fopen(av[1], "r")))
        return clear(fd, NULL, "Error: Operation file corrupted\n");
    if(!(paper = get_paper(fd, &pi)))
        return clear(fd, NULL, "Error: Operation file corrupted\n");
    if (!draw_rectangles(paper, &pi, fd))
        return clear(fd, paper, "Error: Operation file corrupted\n");
    print_paper(paper, &pi);
    clear(fd, paper, NULL);
    return 0;
}