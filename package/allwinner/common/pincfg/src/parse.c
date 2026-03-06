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
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "parse.h"
#define BUFFER_SIZE 1024

#if X_GPIO_BASE_ADDR == 0x0300b000 //如果基地址是813，功能值最大是7(3 bit)
    #define PIN_MAX_VAL 7
#else
    #define PIN_MAX_VAL 15
#endif

#ifndef X_GPIO_OS_SEL_OFFSET
#define X_GPIO_OS_SEL_OFFSET 0     // 提供默认值
#endif
#ifndef S_GPIO_OS_SEL_OFFSET
#define S_GPIO_OS_SEL_OFFSET 0     // 提供默认值
#endif

// 提取字符串中第 col_num 个制表符分隔字段，去除换行符
char* extract_field(const char *line, int col_num) {
    if (!line || col_num <= 0) return NULL;

    char *copy = strdup(line);
    if (!copy) return NULL;

    char *token = strtok(copy, "\t");
    int current_col = 1;

    while (token && current_col < col_num) {
        token = strtok(NULL, "\t");
        current_col++;
    }

    if (!token) {
        free(copy);
        return NULL;
    }

    char *newline = strchr(token, '\n');
    if (newline) *newline = '\0';

    char *result = strdup(token);
    free(copy);
    return result;
}

int calculate_data_register_address(int port_index, char *register_address)
{
    if (port_index < 0 || !register_address)
    {
        fprintf(stderr, "Invalid input");
        return -1;
    }
    int base_address, offset;
    if ((port_index - 10) > 0)
    {
        port_index -= 11;
        base_address = S_GPIO_BASE_ADDR;
        offset = port_index * S_GPIO_PORT_GROUP_SPACING;
    }
    else
    {
        base_address = X_GPIO_BASE_ADDR;
        offset = port_index * X_GPIO_PORT_GROUP_SPACING;
    }
    int additional_offset = GPIO_PORT_DATA_OFFSET;
    int register_address_dec = base_address + offset + additional_offset;

    // 将地址格式化为十六进制字符串
    snprintf(register_address, 32, "0x%x", register_address_dec);
    return 0;
}

int calculate_register_address(int port_index, int gpio_num, char *register_address)
{
    if (port_index < 0 || !register_address)
    {
        fprintf(stderr, "Invalid input");
        return -1;
    }
    int base_address, offset;
    if ((port_index - 10) > 0)
    {
        port_index -= 11;
        base_address = S_GPIO_BASE_ADDR;
        offset = port_index * S_GPIO_PORT_GROUP_SPACING;
    }
    else
    {
        base_address = X_GPIO_BASE_ADDR;
        offset = port_index * X_GPIO_PORT_GROUP_SPACING;
    }
    int reg_index = gpio_num / 8;
    int additional_offset = reg_index * 0x4;
    int register_address_dec = base_address + offset + additional_offset;

    // 将地址格式化为十六进制字符串
    snprintf(register_address, 32, "0x%x", register_address_dec);
    return 0;
}

int calculate_os_sel_register_address(int port_index, int gpio_num, char *register_address)
{
    if (port_index < 0 || !register_address)
    {
        fprintf(stderr, "Invalid input");
        return -1;
    }

    if (X_GPIO_OS_SEL_OFFSET == 0)
       return 1;

    int base_address, offset;
    if ((port_index - 10) > 0)
    {
        port_index -= 11;
        base_address = S_GPIO_BASE_ADDR;
        offset = S_GPIO_OS_SEL_OFFSET + port_index * 0x8;
    }
    else
    {
        base_address = X_GPIO_BASE_ADDR;
        offset = X_GPIO_OS_SEL_OFFSET + port_index * 0x8;
    }
    int reg_index = gpio_num / 16;
    int additional_offset = reg_index * 0x4;
    int register_address_dec = base_address + offset + additional_offset;

    // 将地址格式化为十六进制字符串
    snprintf(register_address, 32, "0x%x", register_address_dec);
    return 0;
}

// 处理配置文件
void process_config_files(const char *config_file_path, const char *output_file_path) {
    FILE *config_file, *output_file;
    int node_fd, node_wfd, fd;
    char line[BUFFER_SIZE];
    const char *node_path = "/sys/class/sunxi_dump/dump";
    char ports[BUFFER_SIZE] = "";
    config_file = fopen(config_file_path, "r");
    if (!config_file) {
        perror("错误：配置文件打开失败");
        return;
    }

    output_file = fopen(output_file_path, "w");
    if (!output_file) {
        perror("错误：输出文件打开失败");
        fclose(config_file);
        return;
    }

    int port_gpio_max[26] = {0};

    // 解析配置文件，确定端口组最大 GPIO 编号和偏移量
    while (fgets(line, sizeof(line), config_file)) {
        char port[3];
        int gpio_num;
        if (sscanf(line, "%2s%d", port, &gpio_num) != 2) { // 将每行分解为2个字符，和一个整数，分别存入port和gpio_num
            fprintf(stderr, "解析失败的行: %s", line);
            continue;
        }

        int port_index = toupper(port[1]) - 'A';

        if (!port_gpio_max[port_index]) {
            port_gpio_max[port_index] = gpio_num;
        } else if (gpio_num > port_gpio_max[port_index]) {
            port_gpio_max[port_index] = gpio_num;
        }

        if (!strstr(ports, port)) {
            strncat(ports, port, sizeof(ports) - strlen(ports) - 1);
            strncat(ports, " ", sizeof(ports) - strlen(ports) - 1);
        }
    }
    rewind(config_file);

    // 遍历每个存在的端口组
    char *current_pos = ports;
    char *port_token = strtok(ports, " ");
    node_fd = open(node_path, O_WRONLY);
    if (node_fd == -1) {
        perror("错误：打开节点文件失败");
        return;
    }
    node_wfd = open(node_path, O_RDONLY);
    if (node_wfd == -1) {
        perror("错误：打开节点文件失败");
        return;
    }
    while (port_token) {
        int port_index, base_address, offset;

        port_index = toupper(port_token[1]) - 'A';
        int gpio_max = port_gpio_max[port_index];
        // 每组的GPIO Data寄存器同一个
        char data_register_address[32];
        calculate_data_register_address(port_index, data_register_address);
        if (write(node_fd, data_register_address, strlen(data_register_address)) == -1) {
            fprintf(output_file, "错误：向 %s 写入 %s 失败。\n", node_path, data_register_address);
        }
        char data_register_value[100];
        lseek(node_wfd, 0, SEEK_SET);
        if (read(node_wfd, data_register_value, sizeof(data_register_value)) == -1) {
            close(node_wfd);
            fprintf(output_file, "错误：从 %s 读取 %s 返回值失败。\n", node_path, data_register_address);
        }
        char *data_endptr;
        long long data_decimal_value = strtoll(data_register_value, &data_endptr, 16);
        if (*data_endptr != '\n') {
            fprintf(output_file, "错误：从 %s 读取的 %s 不是有效的十六进制数。\n", node_path, data_register_value);
        }

        // 遍历端口组的每个 GPIO
        for (int gpio_num = 0; gpio_num <= gpio_max; gpio_num++) {
            char register_address[32];
            calculate_register_address(port_index, gpio_num, register_address);
            if (write(node_fd, register_address, strlen(register_address)) == -1) {
                fprintf(output_file, "错误：向 %s 写入 %s（%s%d）失败。\n", node_path, register_address, port_token, gpio_num);
                continue;
            }

            char register_value[100];
            lseek(node_wfd, 0, SEEK_SET);
            if (read(node_wfd, register_value, sizeof(register_value)) == -1) {
                close(node_wfd);
                fprintf(output_file, "错误：从 %s 读取 %s%d 返回值失败。\n", node_path, port_token, gpio_num);
                continue;
            }
            char *endptr;
            long long decimal_value = strtoll(register_value, &endptr, 16);
            if (*endptr != '\n') {
                fprintf(output_file, "错误：从 %s 读取的 %s 不是有效的十六进制数。\n", node_path, register_value);
                continue;
            }
            int start_bit = (gpio_num % 8) * 4;
            long long mask = 0xF << start_bit;
            long long result = (decimal_value & mask) >> start_bit;
            int gpio_data = 0;

            rewind(config_file);
            char match_line[BUFFER_SIZE];
            char matched_str[BUFFER_SIZE];
            while (fgets(match_line, sizeof(match_line), config_file)) {
                char temp_port[3];
                int temp_gpio_num;
                if (sscanf(match_line, "%2s%d", temp_port, &temp_gpio_num) == 2 &&
                    strcmp(temp_port, port_token) == 0 && temp_gpio_num == gpio_num) {
                    break;
                }
            }

            switch (result) {
                case 0:
                    strcpy(matched_str, "Input");
                    break;
                case 1:
                    strcpy(matched_str, "Output");
                    break;
                case PIN_MAX_VAL-1:
                    snprintf(matched_str, sizeof(matched_str), "%c-EINT%d", port_token[0], gpio_num);
                    break;
                case PIN_MAX_VAL:
                    strcpy(matched_str, "Disable");
                    break;
                default:
                    char *token = extract_field(match_line, result);
                    if (token) {
                        strcpy(matched_str, token);
                        matched_str[strcspn(matched_str, "\n")] = 0;
                    } else {
                        strcpy(matched_str, "Reserved");
                    }
                    break;
            }
            gpio_data = (data_decimal_value & (1 << gpio_num)) ? 1 : 0;
            // os_sel
            long long os_sel;
            char os_sel_register_address[32];
            int ret = calculate_os_sel_register_address(port_index, gpio_num, os_sel_register_address);
            if (ret == 1) {
                os_sel = 0;
            } else if (ret == 0) {
                if (write(node_fd, os_sel_register_address, strlen(os_sel_register_address)) == -1) {
                    fprintf(output_file, "错误：向 %s 写入 %s（%s%d）失败。\n", node_path, os_sel_register_address, port_token, gpio_num);
                    continue;
                }
                char os_register_value[100];
                lseek(node_wfd, 0, SEEK_SET);
                if (read(node_wfd, os_register_value, sizeof(os_register_value)) == -1) {
                    close(node_wfd);
                    fprintf(output_file, "错误：从 %s 读取 %s%d 返回值失败。\n", node_path, port_token, gpio_num);
                    continue;
                }
                char *os_endptr;
                long long os_decimal_value = strtoll(os_register_value, &os_endptr, 16);
                if (*endptr != '\n') {
                    fprintf(output_file, "错误：从 %s 读取的 %s 不是有效的十六进制数。\n", node_path, os_register_value);
                    continue;
                }
                int os_start_bit = (gpio_num % 16) * 2;
                long long os_mask = 0x3 << os_start_bit;
                os_sel = (os_decimal_value & os_mask) >> os_start_bit;
            }
            fprintf(output_file, "%s%d\t%s\t%d\t%lld\n", port_token, gpio_num, matched_str, gpio_data, os_sel);
        }
        current_pos = current_pos + strlen(port_token) + 1;
        port_token = strtok(current_pos, " ");
    }
    close(node_fd);
    close(node_wfd);
    fclose(config_file);
    fclose(output_file);
    close(node_fd);
}

// 将 GPIO 的 label 转换为寄存器写入地址
void set_gpio_label(const char *config_file_path, const char *gpio_label, const char *label, const char *gpio_data) {
    FILE *config_file;
    int node_fd;
    char line[BUFFER_SIZE];
    int port_index, base_address, offset;
    const char *node_path = "/sys/class/sunxi_dump/dump";
    const char *write_node_path = "/sys/class/sunxi_dump/write";

    char port[3];
    int gpio_num;
    if (sscanf(gpio_label, "%2s%d", port, &gpio_num) != 2) {
        fprintf(stderr, "错误：无法从 %s 中解析出 port 和 gpio_num\n", gpio_label);
        return;
    }

    config_file = fopen(config_file_path, "r");
    if (!config_file) {
        perror("错误：配置文件打开失败");
        return;
    }

    port_index = toupper(port[1]) - 'A';
    char register_address[32];
    calculate_register_address(port_index, gpio_num, register_address);

    int label_value = -1;
    rewind(config_file);
    while (fgets(line, sizeof(line), config_file)) {
        char temp_port[3];
        int temp_gpio_num;
        if (sscanf(line, "%2s%d", temp_port, &temp_gpio_num) == 2 &&
            strcmp(temp_port, port) == 0 && temp_gpio_num == gpio_num) {
            for (int i = 2; i < 16; i++) {
                char *token = extract_field(line, i);
                if (token && strcmp(token, label) == 0) {
                    label_value = i;
                    free(token);
                    break;
                }
                free(token);
            }
            break;
        }
    }
    int data;
    int result = sscanf(gpio_data, "%d",  &data);
    if (result == 1) {
        if (data)
           data = 1;
        else
           data = 0;
    } else {
        perror("错误：gpio_data err");
        return;
    }

    if (label_value == -1) {
        if (!strcmp(label, "Input")) {
            label_value = 0;
        } else if (!strcmp(label, "Output")) {
            label_value = 1;
        } else if (!strncmp(label, port, 1) && strstr(label, "-EINT")) {
            label_value = PIN_MAX_VAL-1;
        } else if (!strcmp(label, "Disable")) {
            label_value = PIN_MAX_VAL;
        }
    }

    if (label_value == -1) {
        fprintf(stderr, "错误：未找到匹配的 label 值。\n");
        fclose(config_file);
        return;
    }

    node_fd = open(node_path, O_WRONLY);
    if (node_fd == -1) {
        perror("错误：打开节点文件失败");
        fclose(config_file);
        return;
    }
    if (write(node_fd, register_address, strlen(register_address)) == -1) {
        perror("错误：向节点写入寄存器地址失败");
        close(node_fd);
        fclose(config_file);
        return;
    }
    close(node_fd);

    char register_value[100];
    node_fd = open(node_path, O_RDONLY);
    lseek(node_fd, 0, SEEK_SET);
    if (read(node_fd, register_value, sizeof(register_value)) == -1) {
        close(node_fd);
        fclose(config_file);
        perror("错误：从节点读取寄存器值失败");
        return;
    }
    close(node_fd);

    char *endptr;
    long long decimal_value = strtoll(register_value, &endptr, 16);
    if (*endptr != '\n') {
        perror("错误：读取的不是有效的十六进制数");
        fclose(config_file);
        return;
    }

    int start_bit = (gpio_num % 8) * 4;
    long long mask = ~(0xF << start_bit);
    decimal_value = (decimal_value & mask) | (label_value << start_bit);

    char new_register_value[32];
    snprintf(new_register_value, sizeof(new_register_value), "0x%llx", decimal_value);

    char new_write_node_value[64];
    snprintf(new_write_node_value, sizeof(new_write_node_value), "%s %s", register_address, new_register_value);

    node_fd = open(write_node_path, O_WRONLY);
    if (write(node_fd, new_write_node_value, strlen(new_write_node_value)) == -1) {
        close(node_fd);
        fclose(config_file);
        perror("错误：向节点写入新的寄存器值失败");
        return;
    }
    close(node_fd);

    // gpio_data
    node_fd = open(node_path, O_WRONLY);
    char data_register_address[32];
    calculate_data_register_address(port_index, data_register_address);
    if (write(node_fd, data_register_address, strlen(data_register_address)) == -1) {
        close(node_fd);
        fclose(config_file);
        return;
    }
    close(node_fd);

    char data_register_value[100];
    node_fd = open(node_path, O_RDONLY);
    lseek(node_fd, 0, SEEK_SET);
    if (read(node_fd, data_register_value, sizeof(data_register_value)) == -1) {
        close(node_fd);
        fclose(config_file);
        return;
    }
    close(node_fd);

    char *data_endptr;
    long long data_decimal_value = strtoll(data_register_value, &data_endptr, 16);
    if (*data_endptr != '\n') {
        fclose(config_file);
        perror("错误：读取的不是有效的十六进制数");
        return;
    }

    if (data)
        data_decimal_value = data_decimal_value | (0x1 << gpio_num);
    else
        data_decimal_value = data_decimal_value & (~(0x1 << gpio_num));

    char new_register_data[32];
    snprintf(new_register_data, sizeof(new_register_data), "0x%llx", data_decimal_value);

    char new_write_node_data[64];
    snprintf(new_write_node_data, sizeof(new_write_node_data), "%s %s", data_register_address, new_register_data);

    node_fd = open(write_node_path, O_WRONLY);
    if (write(node_fd, new_write_node_data, strlen(new_write_node_data)) == -1) {
        close(node_fd);
        fclose(config_file);
        perror("错误：向节点写入新的寄存器值失败");
        return;
    }
    close(node_fd);
    fclose(config_file);
}

