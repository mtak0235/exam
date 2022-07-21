#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int ft_strlen(char const *str) {
	int cnt = 0;
	while (str[cnt])
		cnt++;
	return cnt;	
}
int std_error(char const *str) {
	if (str) 
		return write(1, str, ft_strlen(str));
	return 1;
}
typedef struct s_setting {
	int width;
	int height;
	char bgc;
} t_setting;
typedef struct s_circle {
	char t;
	float cx;
	float cy;
	float r;
	char b;
} t_circle;

int is_right_size(int n) {
	if (!(0 < n && n <= 300))
		return 0;
	return 1;
}

char *get_setting(FILE *fd, t_setting *set) {
	char *paper;
	if (fscanf(fd, "%d %d %c", &set->width, &set->height, &set->bgc))
		return NULL;
	if (!(is_right_size(set->width) && is_right_size(set->height)))
		return NULL;
	if (!(paper = (char *)malloc(sizeof(char) * (set->width * set->height))))
		return NULL;
	for (int i = 0; i < sizeof(set->width * set->height); i++)
		paper[i] = set->bgc;
	return paper;
}

int clear(FILE *fd, char *paper, char *msg) {
	if (fd)
		fclose(fd);
	if (paper)
		free(paper);
	if (msg)
		std_error(msg);
	return 1;
}

int is_in_circle(t_circle *circle, float x, float y) {
	float diff;
	if ((diff = sqrtf(powf(x - circle->cx, 2.) + powf(y - circle->cy, 2.))) > circle->r)
		return 0;
	if (circle->r - diff < 1.)
		return 2;
	return 1;
}

void draw_circle(char *paper, t_circle *circle, t_setting *setting) {
	for (int y = 0; y < setting->height; y++) {
		for (int x = 0; x < setting->width; x++) {
			if ((circle->t == 'c' && is_in_circle(circle, (float)x, (float)y) == 2) 
			|| (circle->t == 'C' && is_in_circle(circle, (float)x, (float)y)))
				paper[y * setting->width + x] = circle->b;
		}
	}
}

int draw_circles(FILE *fd, char *paper, t_setting *settings) {
	t_circle circle;
	int ret;
	while ((ret = fscanf(fd, "%c %f %f %f %c", &circle.t, &circle.cx, &circle.cy, &circle.r, &circle.b))) {
		if (circle.r <= 0.000000000 || (circle.t != 'c' || circle.t != 'C'))
			return 0;
		draw_circle(paper, &circle, settings);
	}
	printf("%d\n", ret);
	//
	return 1;
}

void print_pic(char *paper, t_setting *setting) {
	for (int i = 0; i < setting->height; i++) {
		write(1, paper + i * setting->width, setting->width);
		write(1, "\n", 1);
	}
}
int main(int arg, char **av) {
	t_setting setting;
	char *paper;

	if (arg != 2)
		return std_error("Error: argument\n");
	FILE *fd;
	if (!(fd = fopen(av[1], "r")))
		return std_error("Error: Operation file corrupted 1\n");
	if (!(paper = get_setting(fd, &setting)))
		return clear(fd, NULL, "Error: Operation file corrupted 2\n");
	if (!draw_circles(fd, paper, &setting)) 
		return clear(fd, paper, "Error: Operation file corrupted 3\n");
	print_pic(paper, &setting);
	clear(fd, paper, NULL);
	return 0;
}