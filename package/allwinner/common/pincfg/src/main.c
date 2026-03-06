/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include "parse.h"

#define BOX_EXTRA_SPACE 2
#define BOX_VERTICAL_EXTRA_SPACE 1
#define MODIFIED_LABEL_COLOR_PAIR 2
#define DEFAULT_PINCONFIG "/etc/pinconfig"
#define CONFIRM_BOX_COLOR_PAIR 3
#define MAX_SEARCH_RESULTS_DISPLAY 3// 自定义最大显示行数

// 全局变量，记录分组数量
int group_count;

// 当前高亮的 button 索引
int highlighted = -1;
int highlighted_s = -1;

// 按钮信息结构体
typedef struct {
    char button_label[256];
    const char *gpio_label;
    char *menu_items[256];
    int item_count;
    int label_modified;
    char default_label[256];
    char gpio_data[32];
    bool data;
    int os_sel;
} ButtonInfo;

// 分组信息结构体
typedef struct {
    int start_index;
    int end_index;
    char prefix[10];
    int height;
    int width;
} GroupInfo;

typedef struct {
    int* results;        // 存储匹配结果的索引数组
    int count;           // 总匹配结果数
    int current_index;   // 当前选中结果的索引
    int start_display;   // 当前显示结果的起始索引
} SearchState;

// 信号处理函数，处理 SIGINT 信号
void sigint_handler(int signum) {
    if (signum == SIGINT) {
        clear();
        refresh();
        endwin();
        exit(0);
    }
}

// 初始化搜索状态
void init_search_state(SearchState* state) {
    state->results = NULL;
    state->count = 0;
    state->current_index = 0;
    state->start_display = 0;
}

// 释放搜索状态资源
void free_search_state(SearchState* state) {
    if (state->results) {
        free(state->results);
        state->results = NULL;
    }
    state->count = 0;
}

// 提取 GPIO 标签的前缀
char *get_gpio_prefix(const char *gpio_label) {
    static char prefix[10];
    int i = 0;
    while (gpio_label[i] &&!isdigit(gpio_label[i])) {
        prefix[i] = gpio_label[i];
        i++;
    }
    prefix[i] = '\0';
    return prefix;
}

#define MAX_LINE_LENGTH 256
#define OUTPUT_DIR "/tmp"

// 生成输出文件路径
void generate_output_path(const char *config_file, char *output_config_file) {
    const char *base_name = strrchr(config_file, '/');
    if (base_name == NULL) {
        base_name = config_file;
    } else {
        base_name++;
    }
    snprintf(output_config_file, MAX_LINE_LENGTH, "%s/current_%s", OUTPUT_DIR, base_name);
}

// 从文件读取内容并解析
int read_and_parse_file(const char *filename, ButtonInfo *button_infos) {
    char line[256];
    int button_index = 0;
    char *config_filename;

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open pinconfig");
        return 0;
    }

    config_filename = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
    if (!config_filename) {
        perror("Failed to malloc");
        fclose(file);
        return 0;
    }
    generate_output_path(filename, config_filename);
    FILE *output_file = fopen(config_filename, "w");
    if (!output_file) {
        perror("Failed to open output file");
        free(config_filename);
        fclose(file);
        return 0;
    }
    fclose(output_file);

    process_config_files(filename, config_filename);

    while (fgets(line, sizeof(line), file) && button_index < 256) {
        line[strcspn(line, "\n")] = 0;
        button_infos[button_index].menu_items[0] = strdup("Input");
        button_infos[button_index].menu_items[1] = strdup("Output");
        button_infos[button_index].item_count = 2;
        button_infos[button_index].label_modified = 0;

        char *token = strtok(line, "\t");
        if (token) {
            button_infos[button_index].gpio_label = strdup(token);
            token = strtok(NULL, "\t");
            while (token && button_infos[button_index].item_count < 256) {
                button_infos[button_index].menu_items[button_infos[button_index].item_count++] = strdup(token);
                token = strtok(NULL, "\t");
            }
            strcpy(button_infos[button_index].button_label, "Disable");
            button_infos[button_index].menu_items[button_infos[button_index].item_count++] = strdup("Disable");
            button_index++;
        }
    }
    fclose(file);

    FILE *config_file = fopen(config_filename, "r");
    if (!config_file) {
        for (int i = 0; i < button_index; i++) {
            strcpy(button_infos[i].default_label, "Disable");
            strcpy(button_infos[i].button_label, "Disable");
        }
    } else {
        char config_line[256];
        while (fgets(config_line, sizeof(config_line), config_file)) {
            config_line[strcspn(config_line, "\n")] = 0;
            char *gpio = strtok(config_line, "\t");
            char *label = strtok(NULL, "\t");
            char *data = strtok(NULL, "\t");
            char *os_sel = strtok(NULL, "\t");
            if (gpio && label) {
                for (int i = 0; i < button_index; i++) {
                    if (strcmp(button_infos[i].gpio_label, gpio) == 0) {
                        strcpy(button_infos[i].default_label, label);
                        strcpy(button_infos[i].button_label, label);
                        strcpy(button_infos[i].gpio_data, data);
                        if (strncmp(button_infos[i].gpio_data, "1", 1) == 0)
                            button_infos[i].data = 1;
                        else
                            button_infos[i].data = 0;
                        button_infos[i].os_sel = atoi(os_sel);
                        break;
                    }
                }
            }
        }
        fclose(config_file);
    }
    free(config_filename);
    return button_index;
}

// 计算最大长度，只考虑字符串 / 前的部分
int calculate_max_length(ButtonInfo *button_infos, int num_buttons) {
    int max_length = 0;
    for (int i = 0; i < num_buttons; i++) {
        const char *str;
        // 遍历 button_label
        str = button_infos[i].button_label;
        int len = 0;
        while (str[len] && str[len] != '/') {
            len++;
        }
        if (len > max_length) {
            max_length = len;
        }
        // 遍历 menu_items
        for (int j = 0; j < button_infos[i].item_count; j++) {
            str = button_infos[i].menu_items[j];
            len = 0;
            while (str[len] && str[len] != '/') {
                len++;
            }
            if (len > max_length) {
                max_length = len;
            }
        }
    }
    return max_length;
}

// 分组按钮
int group_buttons(ButtonInfo *button_infos, int num_buttons, GroupInfo *groups) {
    int start_index = 0;
    char prev_prefix[10] = "";
    int max_button_length = calculate_max_length(button_infos, num_buttons);
    int count = 0;

    for (int i = 0; i <= num_buttons; i++) {
        if (i == num_buttons || strcmp(get_gpio_prefix(button_infos[i].gpio_label), prev_prefix)) {
            if (i) {
                strcpy(groups[count].prefix, prev_prefix);
                groups[count].start_index = start_index;
                groups[count].end_index = i - 1;
                groups[count].height = (i - start_index) + 2 * BOX_VERTICAL_EXTRA_SPACE;

                int gpio_max_length = 0;
                for (int j = start_index; j < i; j++) {
                    int gpio_length = strlen(button_infos[j].gpio_label);
                    gpio_max_length = gpio_length > gpio_max_length ? gpio_length : gpio_max_length;
                }
                groups[count].width = max_button_length + 3 + gpio_max_length + 2 * BOX_EXTRA_SPACE;
                count++;
            }
            if (i < num_buttons) {
                strcpy(prev_prefix, get_gpio_prefix(button_infos[i].gpio_label));
                start_index = i;
            }
        }
    }
    return count;
}

// 绘制按钮
void draw_button(WINDOW *win, int y, int x, ButtonInfo *info, int highlighted, int max_button_length) {
    // 清理该行
    wmove(win, y, x);
    whline(win, ' ', 19);

    int label_len = 0;

    if (highlighted) {
        wattron(win, A_REVERSE);
    }
    if (info->label_modified) {
        wattron(win, COLOR_PAIR(MODIFIED_LABEL_COLOR_PAIR));
    }
    if (info->os_sel == 1) {
        mvwprintw(win, y, x, "<Only for RV>");
        label_len = 13;
    } else {
        // 只显示 button_label / 前的部分
        while (info->button_label[label_len] && info->button_label[label_len] != '/') {
            label_len++;
        }
        mvwprintw(win, y, x, "%.*s", label_len, info->button_label);
        // gpio_in or gpio_out
        if (strncmp(info->button_label, "Output", 6) == 0 || strncmp(info->button_label, "Input", 5) == 0) {
            if (info->data) {
                mvwprintw(win, y, x + label_len, "[HIGH]");
                label_len = label_len + 6;
            } else {
                mvwprintw(win, y, x + label_len, "[LOW]");
                label_len = label_len + 5;
            }
        }
    }

    if (info->label_modified) {
        wattroff(win, COLOR_PAIR(MODIFIED_LABEL_COLOR_PAIR));
    }
    if (highlighted) {
        wattroff(win, A_REVERSE);
    }

    // 连接线长度加上 button_label 的长度等于之前的总长度
    int total_length = max_button_length + 1;
    int line_length = total_length - label_len;
    int start_x = x + label_len;
    for (int i = 0; i < line_length; i++) {
        mvwaddch(win, y, start_x + i, '-');
    }
    mvwprintw(win, y, x + total_length, "%s", info->gpio_label);

    if (info->os_sel == 1)
        mvprintw(LINES - 3, 0, "You canot press the ENTER key to change PIN for RV");
    else
        mvprintw(LINES - 3, 0, "                                                  ");

    wrefresh(win);
}

// 计算共同前缀长度
int common_prefix_length(const char *str1, const char *str2) {
    int len = 0;
    while (str1[len] && str2[len] && str1[len] == str2[len] && str1[len] != '/') {
        len++;
    }
    return len;
}

// 绘制菜单，确保 menu_item 加上连接线不超出原区域
void draw_menu(WINDOW *win, int y, int x, ButtonInfo *info, int selected, int max_button_length) {
    wattron(win, COLOR_PAIR(1));
    int common_len = common_prefix_length(info->button_label, info->menu_items[0]);
    for (int i = 0; i < info->item_count; i++) {
        if (i == selected) {
            wattron(win, A_REVERSE);
        }

        // 只显示 menu_item / 前的部分
        const char *item = info->menu_items[i];
        int item_len = 0;
        while (item[item_len] && item[item_len] != '/') {
            item_len++;
        }
        mvwprintw(win, y + i, x, "%.*s", item_len, item);

        if (i == selected) {
            wattroff(win, A_REVERSE);
        }
    }
    wattroff(win, COLOR_PAIR(1));

    // Output 提示信息
    if (strncmp(info->menu_items[selected], "Output", 6) == 0)
        mvprintw(LINES - 4, 0, "You can press the ENTER key to choose HIGH and LOW");
    else
        mvprintw(LINES - 4, 0, "                                                  ");

    wrefresh(win);
}

// 计算可显示分组数
int calculate_groups_on_screen(GroupInfo *groups, int available_rows, int available_cols, int button_x) {
    int current_x = button_x;
    int groups_on_screen = 0;
    for (int g = 0; g < group_count; g++) {
        if (current_x + groups[g].width > available_cols || groups[g].height > available_rows) {
            break;
        }
        groups_on_screen++;
        current_x += groups[g].width + 1;
    }
    return groups_on_screen;
}

// 绘制分组边框
void draw_group_box(WINDOW *win, int y, int x, int height, int width) {
    // 绘制顶部边框
    mvwaddch(win, y, x, ACS_ULCORNER);
    for (int i = 1; i < width - 1; i++) {
        mvwaddch(win, y, x + i, ACS_HLINE);
    }
    mvwaddch(win, y, x + width - 1, ACS_URCORNER);

    // 绘制左右边框
    for (int i = 1; i < height - 1; i++) {
        mvwaddch(win, y + i, x, ACS_VLINE);
        mvwaddch(win, y + i, x + width - 1, ACS_VLINE);
    }

    // 绘制底部边框
    mvwaddch(win, y + height - 1, x, ACS_LLCORNER);
    for (int i = 1; i < width - 1; i++) {
        mvwaddch(win, y + height - 1, x + i, ACS_HLINE);
    }
    mvwaddch(win, y + height - 1, x + width - 1, ACS_LRCORNER);
}

// 绘制屏幕部分
void draw_screen_partially(WINDOW *win, ButtonInfo *button_infos, GroupInfo *groups, int start_group, int end_group, int button_y, int button_x, int max_button_length, int highlighted, int available_rows, int available_cols) {
    werase(win);
    int current_x = button_x;
    for (int g = start_group; g <= end_group; g++) {
        if (current_x + groups[g].width > available_cols || groups[g].height > available_rows) {
            break;
        }
        // 绘制分组边框
        draw_group_box(win, button_y, current_x, groups[g].height, groups[g].width);
        for (int i = groups[g].start_index; i <= groups[g].end_index; i++) {
            int row = button_y + (i - groups[g].start_index) + BOX_VERTICAL_EXTRA_SPACE;
            draw_button(win, row, current_x + BOX_EXTRA_SPACE, &button_infos[i], i == highlighted, max_button_length);
        }
        current_x += groups[g].width + 1;
    }

    if (button_infos[highlighted].os_sel == 1)
        mvprintw(LINES - 3, 0, "You canot press the ENTER key to change PIN for RV");
    else
        mvprintw(LINES - 3, 0, "                                                  ");

    wrefresh(win);
}

// 释放按钮信息内存
void free_button_info(ButtonInfo *button_infos, int num_buttons) {
    for (int i = 0; i < num_buttons; i++) {
        free((char *)button_infos[i].gpio_label);
        for (int j = 0; j < button_infos[i].item_count; j++) {
            free(button_infos[i].menu_items[j]);
        }
    }
}

// 获取按钮所在组索引
int get_group_index(GroupInfo *groups, int button_index) {
    for (int g = 0; g < group_count; g++) {
        if (groups[g].start_index <= button_index && button_index <= groups[g].end_index) {
            return g;
        }
    }
    return -1;
}

// 获取按钮 y 坐标
int get_button_y_on_screen(ButtonInfo *button_infos, GroupInfo *groups, int button_y, int button_index) {
    int group_index = get_group_index(groups, button_index);
    return button_y + (button_index - groups[group_index].start_index) + BOX_VERTICAL_EXTRA_SPACE;
}

// 获取按钮在当前屏幕的 x 坐标
int get_button_x_on_screen(GroupInfo groups[], int group_count, int start_group, int button_index, int button_x) {
    int group_index = get_group_index(groups, button_index);
    if (group_index == -1) {
        return -1; // 未找到对应的分组
    }

    int current_x = button_x;
    for (int g = start_group; g < group_index; g++) {
        current_x += groups[g].width + 1;
    }
    return current_x + BOX_EXTRA_SPACE;
}

// 判断是否能放下最大组
int can_fit_largest_group(GroupInfo *groups, int available_rows, int available_cols, int button_x) {
    int max_width = 0, max_height = 0;
    for (int g = 0; g < group_count; g++) {
        max_width = groups[g].width > max_width ? groups[g].width : max_width;
        max_height = groups[g].height > max_height ? groups[g].height : max_height;
    }
    return button_x + max_width <= available_cols && max_height <= available_rows;
}

// 调整显示区域，使高亮按钮可见
int adjust_display(GroupInfo *groups, int *start_group, int highlighted, int groups_on_screen) {
    int highlighted_group = get_group_index(groups, highlighted);
    int change = 0;
    while (highlighted_group < *start_group) {
        (*start_group)--;
        change = 1;
    }
    while (highlighted_group >= *start_group + groups_on_screen) {
        (*start_group)++;
        change = 1;
    }
    return change;
}
// 新增绘制确认框函数
void draw_confirmation_box(WINDOW *win, int rows, int cols, int selected) {
    const char *message = "Do you wish to save your new configuration";
    const char *yes_label = "< yes >";
    const char *no_label = "< no >";

    int box_height = 6;
    int box_width = strlen(message) + 4;  // 适当增加宽度
    int box_y = (rows - box_height) / 2;
    int box_x = (cols - box_width) / 2;

    werase(win);
    // 绘制灰色边框
    wattron(win, COLOR_PAIR(CONFIRM_BOX_COLOR_PAIR));
    draw_group_box(win, box_y, box_x, box_height, box_width);
    wattroff(win, COLOR_PAIR(CONFIRM_BOX_COLOR_PAIR));

    // 绘制提示信息
    mvwprintw(win, box_y + 2, box_x + (box_width - strlen(message)) / 2, "%s", message);

    // 绘制按钮
    int button_y = box_y + 4;
    int yes_x = box_x + (box_width - strlen(yes_label) - strlen(no_label) - 3) / 2;
    int no_x = yes_x + strlen(yes_label) + 3;

    if (selected == 0) {
        wattron(win, A_REVERSE);
    }
    mvwprintw(win, button_y, yes_x, "%s", yes_label);
    if (selected == 0) {
        wattroff(win, A_REVERSE);
    }

    if (selected == 1) {
        wattron(win, A_REVERSE);
    }
    mvwprintw(win, button_y, no_x, "%s", no_label);
    if (selected == 1) {
        wattroff(win, A_REVERSE);
    }

    wrefresh(win);
}

// 绘制确认界面
void draw_confirmation_screen(WINDOW *win, int rows, int cols, int selected) {
    werase(win);
    int y = rows / 2 - 1;
    int x = cols / 2 - 15;
    mvwprintw(win, y, x, "Do you wish to save your new configuration");
    y += 2;
    x = cols / 2 - 5;
    if (selected == 0) {
        wattron(win, A_REVERSE);
    }
    mvwprintw(win, y, x, "Yes");
    if (selected == 0) {
        wattroff(win, A_REVERSE);
    }
    x += 10;
    if (selected == 1) {
        wattron(win, A_REVERSE);
    }
    mvwprintw(win, y, x, "No");
    if (selected == 1) {
        wattroff(win, A_REVERSE);
    }
    wrefresh(win);
}

void save_configuration(ButtonInfo *button_infos, int num_buttons) {
    for (int i = 0; i < num_buttons; i++) {
        if (button_infos[i].label_modified) {
            if (strncmp(button_infos[i].button_label, "Output", 6) == 0) {
                if (button_infos[i].data)
                    strcpy(button_infos[i].gpio_data, "1");
                else
                    strcpy(button_infos[i].gpio_data, "0");
            }
            set_gpio_label(DEFAULT_PINCONFIG, button_infos[i].gpio_label, button_infos[i].button_label, button_infos[i].gpio_data);
        }
    }
}

// 搜索匹配的按钮
void search_buttons(SearchState* state, ButtonInfo *button_infos, int num_buttons, const char *keyword) {
    if (!state || !button_infos) return;
    free_search_state(state);

    int* temp_results = NULL;
    int match_count = 0;
    // 第一次遍历统计匹配数量
    for (int i = 0; i < num_buttons; i++) {
        if (strcasestr(button_infos[i].button_label, keyword) ||
            strcasestr(button_infos[i].gpio_label, keyword)) {
            match_count++;
        }
    }

    if (match_count > 0) {
        temp_results = malloc(match_count * sizeof(int));
        if (!temp_results) {
            perror("Failed to allocate memory for search results");
            return;
        }

        // 第二次遍历记录索引
        int index = 0;
        for (int i = 0; i < num_buttons; i++) {
            if (strcasestr(button_infos[i].button_label, keyword) ||
                strcasestr(button_infos[i].gpio_label, keyword)) {
                temp_results[index++] = i;
            }
        }
    }

    state->results = temp_results;
    state->count = match_count;
}

// 绘制搜索结果
void draw_search_results(SearchState* state, WINDOW *win, ButtonInfo *button_infos, int max_button_length, int input_row) {
    if (!state || !button_infos) {
        return;
    }
    int end_display = state->start_display + MAX_SEARCH_RESULTS_DISPLAY;
    if (end_display > state->count) {
        end_display = state->count;
    }
    for (int i = state->start_display; i < end_display; i++) {
        int display_index = i - state->start_display;
        int button_index = state->results[i];
        ButtonInfo *info = &button_infos[button_index];
        int high = (i == state->current_index);
        draw_button(win, input_row - (end_display - i), 0, info, high, max_button_length);
    }
    //wrefresh(win);
}

// 处理搜索结果的上下遍历
void handle_search_navigation(SearchState* state, ButtonInfo *button_infos, WINDOW *win, int input_row, int max_button_length) {
    if (!state || !button_infos) {
        return;
    }

    int ch;
    while (1) {
        ch = getch();
        if (ch == '\n') {
            // 清除搜索结果显示
            for (int i = 0; i < MAX_SEARCH_RESULTS_DISPLAY; i++)
                mvprintw(input_row - 1 - i, 0, "%*s", 20, "");
            break;
        }

        if (ch == KEY_UP) {
            if (state->current_index > 0) {
                state->current_index--;
                if (state->current_index < state->start_display) {
                    state->start_display--;
                }
            }
        } else if (ch == KEY_DOWN) {
            if (state->current_index < state->count - 1) {
                state->current_index++;
                if (state->current_index >= state->start_display + MAX_SEARCH_RESULTS_DISPLAY) {
                    state->start_display++;
                }
            }
        }
        draw_search_results(state, win, button_infos, max_button_length, input_row);
    }
}

int main(int argc, char *argv[]) {
    setenv("ESCDELAY", "25", 1);
    setenv("LINES", "40", 1);
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(CONFIRM_BOX_COLOR_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(MODIFIED_LABEL_COLOR_PAIR, COLOR_GREEN, COLOR_BLACK);
    signal(SIGINT, sigint_handler);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    int available_rows = rows - 1, available_cols = cols - 1;
    SearchState state;
    init_search_state(&state);
    ButtonInfo button_infos[256];
    int num_buttons = read_and_parse_file(DEFAULT_PINCONFIG, button_infos);
    if (!num_buttons) {
        endwin();
        fprintf(stderr, "No valid button information found in the file.\n");
        return 1;
    }

    GroupInfo groups[256];
    group_count = group_buttons(button_infos, num_buttons, groups);

    int button_y = 1, button_x = 1;
    int max_button_length = calculate_max_length(button_infos, num_buttons);

    if (!can_fit_largest_group(groups, available_rows, available_cols, button_x)) {
        endwin();
        fprintf(stderr, "The screen cannot fit the largest group. Exiting...\n");
        return 1;
    }

    int start_group = 0;
    int highlighted = 0;
    int menu_open = 0;
    int submenu_open = 0;
    int menu_selected = 0;
    int groups_on_screen = calculate_groups_on_screen(groups, available_rows, available_cols, button_x);
    if (!groups_on_screen) {
        endwin();
        fprintf(stderr, "Not enough space on the screen to display any groups.\n");
        return 1;
    }

    draw_screen_partially(stdscr, button_infos, groups, start_group, start_group + groups_on_screen - 1, button_y, button_x, max_button_length, highlighted, available_rows, available_cols);

    int ch;
    int change;
    int old_highlighted;
    int confirm_screen = 0;
    int confirm_selected = 0;
    int input_row = available_rows - 1;
    char keyword[256];
    while ((ch = getch()) != 'q') {
        old_highlighted = highlighted;
        if (confirm_screen) {
            switch (ch) {
                case KEY_LEFT:
                    confirm_selected = (confirm_selected - 1 + 2) % 2;
                    draw_confirmation_box(stdscr, rows, cols, confirm_selected);
                    break;
                case KEY_RIGHT:
                    confirm_selected = (confirm_selected + 1) % 2;
                    draw_confirmation_box(stdscr, rows, cols, confirm_selected);
                    break;
                case KEY_ENTER:
                case '\n':
                    if (confirm_selected == 0) {
                        // 保存配置的逻辑
                        save_configuration(button_infos, num_buttons);
                    }
                    confirm_screen = 0;
                    clear();
                    refresh();
                    endwin();
                    exit(0);
                case 27: // ESC 键，关闭确认界面
                    confirm_screen = 0;
                    clear();
                    refresh();
                    endwin();
                    exit(0);
            }
        } else {
            int nosearch_len = strlen(keyword) + strlen("[] not found\n");
            mvprintw(input_row, 0, "%*s", nosearch_len, "");
            refresh();
            switch (ch) {
                case '\\':
                case '/':
                case 's':
                    echo();
                    mvprintw(input_row, 0, "Search: ");
                    getstr(keyword);
                    noecho();
                    search_buttons(&state, button_infos, num_buttons, keyword);
                    draw_search_results(&state, stdscr, button_infos, max_button_length, input_row);
                    if (state.count > 0) {
                        handle_search_navigation(&state, button_infos, stdscr, input_row, max_button_length);
                        highlighted = state.results[state.current_index];
                        int clear_len = strlen(keyword) + strlen("Search: ");
                        mvprintw(input_row, 0, "%*s", clear_len, "");
                        refresh();
                    } else
                        mvprintw(input_row, 0, "[%s] not found\n", keyword);
                    break;
                case KEY_UP:
                    if (menu_open) {
                        menu_selected = (menu_selected - 1 + button_infos[highlighted].item_count) % button_infos[highlighted].item_count;
                        int current_y = get_button_y_on_screen(button_infos, groups, button_y, highlighted);
                        int current_x = get_button_x_on_screen(groups, group_count, start_group, highlighted, button_x);
                        draw_menu(stdscr, current_y + 1, current_x, &button_infos[highlighted], menu_selected, max_button_length);
                    } else {
                        int current_group = get_group_index(groups, highlighted);
                        if (highlighted == groups[current_group].start_index) {
                            if (current_group > 0) {
                                highlighted = groups[current_group - 1].end_index;
                            }
                        } else {
                            highlighted--;
                        }
                    }
                    break;
                case KEY_DOWN:
                    if (menu_open) {
                        menu_selected = (menu_selected + 1) % button_infos[highlighted].item_count;
                        int current_y = get_button_y_on_screen(button_infos, groups, button_y, highlighted);
                        int current_x = get_button_x_on_screen(groups, group_count, start_group, highlighted, button_x);
                        draw_menu(stdscr, current_y + 1, current_x, &button_infos[highlighted], menu_selected, max_button_length);
                    } else {
                        int current_group = get_group_index(groups, highlighted);
                        if (highlighted == groups[current_group].end_index) {
                            if (current_group < group_count - 1) {
                                highlighted = groups[current_group + 1].start_index;
                            }
                        } else {
                            highlighted++;
                        }
                    }
                    break;
                case KEY_ENTER:
                case '\n':
                    if (button_infos[highlighted].os_sel == 1) {
                        break;
                    }
                    if (menu_open) {
                        const char *selected_item = button_infos[highlighted].menu_items[menu_selected];
                        int len = 0;
                        while (selected_item[len] && selected_item[len] != '/') {
                            len++;
                        }
                        strncpy(button_infos[highlighted].button_label, selected_item, len);
                        button_infos[highlighted].button_label[len] = '\0';

                        if (submenu_open) {
                            if (strncmp(button_infos[highlighted].button_label, "Output", 6) == 0) {
                                int current_y = get_button_y_on_screen(button_infos, groups, button_y, highlighted);
                                int current_x = get_button_x_on_screen(groups, group_count, start_group, highlighted, button_x);
                                bool high = 0;
                                bool old_high = 0;
                                if (button_infos[highlighted].data)
                                    high = 1;
                                else
                                    high = 0;

                                if (strncmp(button_infos[highlighted].gpio_data, "1", 1) == 0)
                                    old_high = 1;
                                else
                                    old_high = 0;
                                wattron(stdscr, COLOR_PAIR(3));
                                if (high)
                                    mvprintw(current_y + 3, current_x + 15, "LOW");
                                else
                                    mvprintw(current_y + 2, current_x + 15, "HIGH");
                                wattroff(stdscr, COLOR_PAIR(3));
                                wattron(stdscr, A_REVERSE | COLOR_PAIR(3));
                                if (high)
                                    mvprintw(current_y + 2, current_x + 15, "HIGH");
                                else
                                    mvprintw(current_y + 3, current_x + 15, "LOW");
                                wattroff(stdscr, A_REVERSE | COLOR_PAIR(3));
                                int submenu_active = 1;
                                button_infos[highlighted].label_modified = 0;
                                while (submenu_active) {
                                    int ch = getch();
                                    switch (ch) {
                                        case KEY_UP:
                                        case KEY_DOWN:
                                            high = !high;
                                            wattron(stdscr, A_REVERSE | COLOR_PAIR(3));
                                            // choose true one
                                            if (high)
                                                mvprintw(current_y + 2, current_x + 15, "HIGH");
                                            else
                                                mvprintw(current_y + 3, current_x + 15, "LOW");
                                            wattroff(stdscr, A_REVERSE | COLOR_PAIR(3));
                                            wattron(stdscr, COLOR_PAIR(3));
                                            if (high)
                                                mvprintw(current_y + 3, current_x + 15, "LOW");
                                            else
                                                mvprintw(current_y + 2, current_x + 15, "HIGH");
                                            wattroff(stdscr, COLOR_PAIR(3));
                                            break;
                                        case KEY_ENTER:
                                        case '\n':
                                        case 27: // ESC
                                            submenu_active = 0;
                                            break;
                                    }
                                }
                                submenu_open = 0;
                                menu_open = 0;
                                if (high != old_high) {
                                    button_infos[highlighted].label_modified = 1;
                                } else {
                                    if (strcmp(button_infos[highlighted].button_label, button_infos[highlighted].default_label) != 0)
                                        button_infos[highlighted].label_modified = 1;
                                }
                                button_infos[highlighted].data = high;
                                draw_screen_partially(stdscr, button_infos, groups, start_group, start_group + groups_on_screen - 1, button_y, button_x, max_button_length, highlighted, available_rows, available_cols);
                                break;
                            }
                        }
                        // 比较修改后的标签与默认标签
                        if (strcmp(button_infos[highlighted].button_label, button_infos[highlighted].default_label) != 0) {
                            button_infos[highlighted].label_modified = 1;
                        } else {
                            button_infos[highlighted].label_modified = 0;
                        }

                        menu_open = 0;
                        draw_screen_partially(stdscr, button_infos, groups, start_group, start_group + groups_on_screen - 1, button_y, button_x, max_button_length, highlighted, available_rows, available_cols);
                    } else {
                        // 清空屏幕
                        clear();
                        refresh();
                        // 获取当前高亮按钮的位置
                        int current_y = get_button_y_on_screen(button_infos, groups, button_y, highlighted);
                        int current_x = get_button_x_on_screen(groups, group_count, start_group, highlighted, button_x);
                        // 重新绘制当前按钮的连接线和 gpio_label
                        draw_button(stdscr, current_y, current_x, &button_infos[highlighted], 0, max_button_length);
                        menu_open = 1;
                        submenu_open = 1;
                        // 找到与 button_label 匹配的菜单项
                        for (int i = 0; i < button_infos[highlighted].item_count; i++) {
                            int label_len = 0;
                            while (button_infos[highlighted].button_label[label_len] && button_infos[highlighted].button_label[label_len] != '/') {
                                label_len++;
                            }
                            int item_len = 0;
                            while (button_infos[highlighted].menu_items[i][item_len] && button_infos[highlighted].menu_items[i][item_len] != '/') {
                                item_len++;
                            }
                            if (label_len == item_len && strncmp(button_infos[highlighted].button_label, button_infos[highlighted].menu_items[i], label_len) == 0) {
                                menu_selected = i;
                                break;
                            }
                        }
                        draw_menu(stdscr, current_y + 1, current_x, &button_infos[highlighted], menu_selected, max_button_length);
                    }
                    break;
                case 27: // ESC 键
                    confirm_screen = 1;
                    confirm_selected = 0;
                    werase(stdscr);
                    draw_confirmation_box(stdscr, rows, cols, confirm_selected);
                    break;
            }
        }
        if (!confirm_screen) {
            change = adjust_display(groups, &start_group, highlighted, groups_on_screen);
            if (change) {
                if (!menu_open) {
                    draw_screen_partially(stdscr, button_infos, groups, start_group, start_group + groups_on_screen - 1, button_y, button_x, max_button_length, highlighted, available_rows, available_cols);
                }
            } else {
                int current_y = get_button_y_on_screen(button_infos, groups, button_y, old_highlighted);
                int current_x = get_button_x_on_screen(groups, group_count, start_group, old_highlighted, button_x);
                draw_button(stdscr, current_y, current_x, &button_infos[old_highlighted], 0, max_button_length);
                current_y = get_button_y_on_screen(button_infos, groups, button_y, highlighted);
                current_x = get_button_x_on_screen(groups, group_count, start_group, highlighted, button_x);
                draw_button(stdscr, current_y, current_x, &button_infos[highlighted], 1, max_button_length);
            }
        }
    }

    free_button_info(button_infos, num_buttons);
    endwin();
    return 0;
}
