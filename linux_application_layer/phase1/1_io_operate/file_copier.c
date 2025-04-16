/****** 文本复制器 ******/
#include <stdio.h>      // 标准输入输出库
#include <stdlib.h>     // 提供 EXIT_SUCCESS 和 EXIT_FAILURE
#include <fcntl.h>      // 文件控制选项（如 O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC）
#include <unistd.h>     // 提供 `read`, `write`, `close` 等系统调用

#define BUFFER_SIZE 1024  // 定义缓冲区大小，每次读取 1024 字节，提高效率

int main(int argc, char *argv[])
{
    // 检查命令行参数是否正确（程序名 + 2 个参数）
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return EXIT_FAILURE;  // 参数错误，退出程序
    }

    const char *source_file = argv[1];      // 获取源文件名
    const char *destination_file = argv[2]; // 获取目标文件名

    // 以只读模式打开源文件
    int source_fd = open(source_file, O_RDONLY);
    if (source_fd == -1) {      // 检查是否打开失败
        perror("Failed to open source file");   // 输出错误信息
        return EXIT_FAILURE;    // 退出程序
    }

    // 以写入模式打开目标文件（如果不存在则创建，如果存在则清空）
    int destination_fd = open(destination_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination_fd == -1) {  // 检查是否打开失败
        perror("Failed to open destination file");
        close(source_fd);  // 关闭源文件，防止资源泄漏
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];  // 用于存储读取的文件内容
    ssize_t bytes_read;        // 存储 `read` 返回的字节数

    // 逐块读取源文件，并写入目标文件
    while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
        ssize_t bytes_written = write(destination_fd, buffer, bytes_read);  // 写入文件
        if (bytes_written == -1) {  // 写入失败
            perror("Failed to write to destination file");
            close(source_fd);
            close(destination_fd);
            return EXIT_FAILURE;
        }
        if (bytes_written != bytes_read) {  // 检查是否完全写入
            fprintf(stderr, "Warning: Written bytes do not match read bytes\n");
        }
    }

    // 检查 `read` 是否遇到错误
    if (bytes_read == -1) {
        perror("Failed to read from source file");
        close(source_fd);
        close(destination_fd);
        return EXIT_FAILURE;
    }

    // 关闭源文件
    if (close(source_fd) == -1) {
        perror("Failed to close source file");
        close(destination_fd);
        return EXIT_FAILURE;
    }

    // 关闭目标文件
    if (close(destination_fd) == -1) {
        perror("Failed to close destination file");
        return EXIT_FAILURE;
    }

    printf("File copied successfully.\n");  // 复制成功
    return EXIT_SUCCESS;
}
