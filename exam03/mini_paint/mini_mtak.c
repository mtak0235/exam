#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
typedef struct s_paper {
	int width;
	int height;
	char c;
} t_paper;

typedef struct s_circle {
	char t;
	float cx;
	float cy;
	float r;
	char b;
} t_circle;

int ft_strlen(char *str) {
	int cnt = 0;
	while (str[cnt])
		cnt++;
	return cnt;
}

int ft_err(char *msg) {
	if (msg)
		write(1, msg, ft_strlen(msg));
	return 1;
}

int is_valid_size(int n) {
	if (!(0 < n && n <= 300))
		return 0;
	return 1;
}
char *get_paper(FILE *fd, t_paper *paper_info) {
	char *paper;
	if (fscanf(fd, "%d %d %c\n", &paper_info->width, &paper_info->height, &paper_info->c) != 3)
		return 0;
	if (!is_valid_size(paper_info->width) && !is_valid_size(paper_info->height))
		return 0;
	if (!(paper = (char *)malloc(sizeof(char) * (paper_info->height * paper_info->width))))
		return 0;
	for (int i = 0; i < (paper_info->width * paper_info->height); i++)
		paper[i] = paper_info->c;
	return paper;
}

int clear(FILE *fd, char *paper, char *msg) {
	if (fd)
		fclose(fd);
	if (paper)
		free(paper);
	if (msg)
		ft_err(msg);
	return 1;
}

int is_paintable(float x, float y, t_circle *circle) {
	float distance = sqrtf(powf(circle->cx - x, 2.) + powf(circle->cy -y, 2.));
	if (distance > circle->r)
		return 0;
	if (circle->t == 'C')
		return 1;
	if (circle->r - distance > 1.)
		return 0;
	return 1;
}

void draw_circle(char *paper, t_circle *circle, t_paper *pi) {
	for (int y = 0; y < pi->height; y++) {
		for (int x = 0; x < pi->width; x++) {
			if (is_paintable((float)x, (float)y, circle))
				paper[y * pi->width + x] = circle->b;
		}
	}
}

int draw_circles(char *paper, t_paper *pi, FILE *fd) {
	t_circle circle;
	int ret;
	while ((ret = fscanf(fd, "%c %f %f %f %c\n", &circle.t, &circle.cx,&circle.cy,&circle.r, &circle.b)) == 5) {
		if (!(circle.t == 'c' || circle.t == 'C') || circle.r <= 0.)
			return 0;
		draw_circle(paper, &circle, pi);
	}
	return ret == -1? 1 : 0;
}

void print_paper(char *paper, t_paper *pi) {
	for (int i = 0; i < pi->height; i++) {
		write(1, &paper[i * pi->width], pi->width);
		write(1,"\n", 1);
	}
}

int main(int ag, char **av) {
	FILE *fd;
	t_paper paper_info;
	char *paper;

	if (ag !=  2)
		return ft_err("Error: argument\n");
	if (!(fd = fopen(av[1],"r")))
		return ft_err("Error: Operation file corrupted\n");
	if (!(paper = get_paper(fd, &paper_info)))
		return clear(fd, NULL, "Error: Operation file corrupted\n");
	if (!draw_circles(paper, &paper_info, fd))
		return clear(fd, paper, "Error: Operation file corrupted\n");
	print_paper(paper, &paper_info);
	clear(fd, paper, NULL);
	return 0;
}