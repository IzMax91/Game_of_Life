#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define X 80
#define Y 25
#define CHAR_LEN 256
#define COUNT_FIGURES 5

void editor(char array[Y][X], int *px, int *py);
void rendering(char array[Y][X], int px, int py, char mode, int generation);
void alive_or_dead(char array[Y][X], char array_new[Y][X]);
void new_array(char array_inp[Y][X], char array_out[Y][X]);
void *increase_speed(void *sp);

void insert_figure(char array[Y][X], int *player_x, int *player_y);
void string_comparison(char string_1[CHAR_LEN], const char string_2[CHAR_LEN], int *error);
void string_len(const char arr[CHAR_LEN], int *length);
void find_figure_name(char arr[CHAR_LEN], int *error);
void open_figure(char array[Y][X], int *player_x, int *player_y, char name_figure[CHAR_LEN]);
void string_copy(char string_1[CHAR_LEN], char string_2[CHAR_LEN]);
void add_figure(char array[Y][X], char shape[Y][X], const int *player_x, const int *player_y, int shape_rows,
                int shape_cols);
void string_concat(char *string_2, const char *string_1);
void header(char mode, int generation);
void print_field_terminal(char field_old[Y][X], int player_x, int player_y, char mode, int generation);
void print_field_from_file(char field_old[Y][X], int player_x, int player_y, char mode, int generation);
void end_life();

int main() {
    char field_old[Y][X];
    char field_new[Y][X];
    int player_x = X / 2, player_y = Y / 2;
    int generation = 0;
    int end_game = 0;
    int speed = 250000;
    pthread_t thread;

    if (isatty(STDIN_FILENO)) {
        print_field_terminal(field_old, player_x, player_y, 'e', 0);
        editor(field_old, &player_x, &player_y);
    } else {
        print_field_from_file(field_old, player_x, player_y, 'e', 0);
        if ((freopen("/dev/tty", "r", stdin)) != NULL) system("stty -icanon -echo");
    }

    pthread_create(&thread, NULL, increase_speed, &speed);

    while (!end_game) {
        new_array(field_old, field_new);
        alive_or_dead(field_old, field_new);

        rendering(field_new, -1, -1, 'g', generation);
        new_array(field_new, field_old);

        if (speed != -1) {
            generation += 1;
            usleep(speed);
        } else {
            end_game = 1;
            end_life();
        }
    }
    return 0;
}

void *increase_speed(void *sp) {
    int *speed = sp;

    while (1) {
        char ent = getchar();
        if (ent == '=' && *speed >= 50001)
            *speed -= 50000;
        else if (ent == '-' && *speed < 500000)
            *speed += 50000;
        else if (ent == 'q') {
            *speed = -1;
            pthread_exit(increase_speed);
        }
    }
}

void print_field_terminal(char field_old[Y][X], int player_x, int player_y, char mode, int generation) {
    for (int i = 0; i < Y; i++)
        for (int j = 0; j < X; j++) {
            field_old[i][j] = '.';
        }
    rendering(field_old, player_x, player_y, mode, generation);
}

void print_field_from_file(char field_old[Y][X], int player_x, int player_y, char mode, int generation) {
    for (int i = 0; i < Y; i++)
        for (int j = 0; j < X; j++) {
            field_old[i][j] = getchar();
        }
    rendering(field_old, player_x, player_y, mode, generation);
}

void insert_figure(char array[Y][X], int *player_x, int *player_y) {
    system("stty sane");
    char name_figure[CHAR_LEN];
    int flag = 0;
    int error;

    while (!flag) {
        error = 0;
        flag = 1;

        printf(
            "Для добавления фигуры на поле введите ее название.\nДоступные "
            "фигуры:\n\n1) glider\n2) clock\n3) square\n4) pentomino\n5) "
            "galactic\n");
        scanf("%99s", name_figure);

        find_figure_name(name_figure, &error);

        if (!error) {
            open_figure(array, player_x, player_y, name_figure);
        } else {
            printf("\nТакой фигуры не найдено!\nПопробуйте еще!\n");
            flag = 0;
        }
    }

    system("stty -icanon -echo");
}

void open_figure(char array[Y][X], int *player_x, int *player_y, char name_figure[CHAR_LEN]) {
    char name_folder[] = "figures/";
    FILE *shape_file;
    string_concat(name_figure, ".txt");
    string_concat(name_folder, name_figure);

    if ((shape_file = fopen(name_folder, "r")) == NULL) {
        printf("\nНе получилось открыть файл..\n");
    } else {
        int shape_rows = 0;
        int shape_cols = 0;
        char shape[Y][X];
        int length_line;
        char line[CHAR_LEN];

        while (fgets(line, sizeof(line), shape_file)) {
            string_copy(shape[shape_rows], line);
            string_len(line, &length_line);
            shape_rows += 1;
            shape_cols = length_line - 1;
        }

        fclose(shape_file);

        add_figure(array, shape, player_x, player_y, shape_rows, shape_cols);
    }
}

void add_figure(char array[Y][X], char shape[Y][X], const int *player_x, const int *player_y, int shape_rows,
                int shape_cols) {
    int shape_x = *player_x;
    int shape_y = *player_y;

    for (int i = 0; i < shape_rows; i++) {
        for (int j = 0; j < shape_cols; j++) {
            if (shape[i][j] == 'X') {
                int array_x = shape_x + i;
                int array_y = shape_y + j;
                if (array_x >= 0 && array_x < X && array_y >= 0 && array_y < Y) {
                    array[array_y][array_x] = '#';
                }
            }
        }
    }
}

void string_copy(char string_1[CHAR_LEN], char string_2[CHAR_LEN]) {
    while (*string_2 != '\0') {
        *string_1 = *string_2;
        string_1 += 1;
        string_2 += 1;
    }
}

void string_concat(char *string_2, const char *string_1) {
    int i, j;

    for (i = 0; string_2[i] != '\0'; i++)
        ;

    for (j = 0; string_1[j] != '\0'; j++) {
        string_2[i + j] = string_1[j];
    }

    string_2[i + j] = '\0';
}

void find_figure_name(char arr[CHAR_LEN], int *error) {
    int error_figure = 0;

    char all_figures_names[COUNT_FIGURES][CHAR_LEN] = {{'g', 'l', 'i', 'd', 'e', 'r'},
                                                       {'c', 'l', 'o', 'c', 'k'},
                                                       {'s', 'q', 'u', 'a', 'r', 'e'},
                                                       {'p', 'e', 'n', 't', 'o', 'm', 'i', 'n', 'o'},
                                                       {'g', 'a', 'l', 'a', 'c', 't', 'i', 'c'}};

    for (int i = 0; i < COUNT_FIGURES; i++) {
        string_comparison(all_figures_names[i], arr, &error_figure);
        if (error_figure == 0) break;
        if (i != COUNT_FIGURES - 1) error_figure = 0;
    }

    *error = error_figure;
}

void string_comparison(char string_1[CHAR_LEN], const char string_2[CHAR_LEN], int *error) {
    int length_string_1 = 0;
    int length_string_2 = 0;

    string_len(string_1, &length_string_1);
    string_len(string_1, &length_string_2);

    if (length_string_1 == length_string_2) {
        for (int i = 0; i < length_string_1; i++) {
            if (string_1[i] != string_2[i]) {
                *error = 1;
                break;
            }
        }
    } else {
        *error = 1;
    }
}

void string_len(const char arr[CHAR_LEN], int *length) {
    char last_char = arr[0];
    int i = 1;

    while (last_char != '\0') {
        last_char = arr[i];
        i += 1;
        *length += 1;
    }
}

void editor(char array[Y][X], int *px, int *py) {
    system("stty -icanon -echo");
    char direction = ' ';

    while (direction != 'q') {
        scanf("%c", &direction);

        if (direction == 'w')
            (*py)--;
        else if (direction == 's')
            (*py)++;
        else if (direction == 'a')
            (*px)--;
        else if (direction == 'd')
            (*px)++;

        if (direction == 'i') {
            insert_figure(array, px, py);
        }

        if (direction == 'e') {
            if (array[*py][*px] == '.')
                array[*py][*px] = '#';
            else
                array[*py][*px] = '.';
        }
        rendering(array, *px, *py, 'e', 0);
    }
}

void alive_or_dead(char array[Y][X], char array_new[Y][X]) {
    int number_y, number_x;

    for (int cell_y = 0; cell_y < Y; cell_y++) {
        for (int cell_x = 0; cell_x < X; cell_x++) {
            int neighbours = 0;
            int alive = 0;
            if (array[cell_y][cell_x] == '#') alive = 1;

            if (cell_y - 1 >= 0)
                number_y = cell_y - 1;
            else
                number_y = Y - 1;
            for (int i = 0; i < 3; i++) {
                if (number_y > Y - 1) number_y = 0;
                if (cell_x - 1 >= 0)
                    number_x = cell_x - 1;
                else
                    number_x = X - 1;

                for (int j = 0; j < 3; j++) {
                    if (number_x > X - 1) number_x = 0;
                    if (array[number_y][number_x] == '#') neighbours++;
                    number_x++;
                }
                number_y++;
            }
            if (alive)
                if (neighbours - 1 == 2 || neighbours - 1 == 3)
                    array_new[cell_y][cell_x] = '#';
                else
                    array_new[cell_y][cell_x] = '.';
            else if (neighbours == 3)
                array_new[cell_y][cell_x] = '#';
        }
    }
}

void rendering(char array[Y][X], int px, int py, char mode, int generation) {
    printf("\033c");
    header(mode, generation);
    for (int i = 0; i < Y; i++) {
        for (int j = 0; j < X; j++) {
            if (i == py && j == px)
                printf("@");
            else
                printf("%c", array[i][j]);
        }
        printf("\n");
    }
}

void new_array(char array_inp[Y][X], char array_out[Y][X]) {
    for (int i = 0; i < Y; i++)
        for (int j = 0; j < X; j++) array_out[i][j] = array_inp[i][j];
}

void header(char mode, int generation) {
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            if (mode == 'e') {
                printf("\t\t\t\t   Редактор");
            } else {
                printf("\t\t\t\tИгра: %d поколение.", generation);
            }
            printf("\n");
        } else {
            for (int j = 0; j < X; j++) {
                printf("=");
            }
            printf("\n");
        }
    }
}

void end_life() {
    printf("\033c");
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            printf("\t\t\t😢 Конец игры, конец жизни 😢\n");
        } else if (i == 0) {
            for (int j = 0; j < X; j++) {
                printf("▲");
            }
            printf("\n");
        } else {
            for (int j = 0; j < X; j++) {
                printf("▼");
            }
            printf("\n");
        }
    }
}
