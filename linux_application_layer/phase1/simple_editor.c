/****** 文本编辑器 ******/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <syslog.h>

#define EXPAND_NUM  100 // 扩展数

// 行 链表
typedef struct line{
    char *content;  // 行内容
    int length;     // 当前行的长度
    int capacity;   // 当前行的最大容量（可动态扩展）
    struct line *next;
}line;

line *text = NULL;  // 动态链表存储文本行
static int line_count = 0;      // 当前使用的行数
static int current_row = 0;     // 当前编辑行
static int current_col = 0;     // 当前编辑列

void printf_line(void)
{
    line *p=text;
    // syslog(LOG_INFO, "line_count:%d行", line_count);
    int i=0;
    while(p) {
        // syslog(LOG_INFO, "链表%d:%s",i,p->content);
        i++;
        p = p->next;
    }
}

// 插入 - 链表
static void insert_text(int index,const char* content)
{
    line * p = text;
    line * node =  (line *)malloc(sizeof(line));
    if(content!=NULL){
        int len = strlen(content);  // 不包含字符串结束符'\0'
        node->content = (char *)malloc((len+1)*sizeof(char));
        strcpy(node->content, content);
        node->length = len;
        node->capacity = len;
    }else{
        node->content = (char *)malloc((EXPAND_NUM+1)*sizeof(char));
        node->length = 0;
        node->content[0] = '\0';
        node->capacity = EXPAND_NUM;
    }
    if(index == 0){         // 头节点
        if(text == NULL){
            node->next =NULL;
            text = node;
        }else{
            free(node->content);
            free(node);
        }
        return;
    }
    while(--index && p->next) {
        p = p->next;
    }
    node->next = p->next;
    p->next = node;
}

// 释放某个结点
static void delete_text(int index)
{
    if (text == NULL) return;  // 链表为空，直接返回

    line *p = text;
    line *q = NULL;

    // 删除头结点
    if (index == 0) {
        text = text->next;
        free(p->content);  // 释放字符串内容
        free(p);           // 释放节点
        return;
    }

    // 遍历找到 index 位置的节点
    while (index-- && p->next) {
        q = p;
        p = p->next;
    }

    if (p) { // 确保 p 不是 NULL
        q->next = p->next;  // 跳过当前节点
        free(p->content);    // 释放字符串内容
        free(p);             // 释放节点
    }
}

// 释放 文本编辑缓冲区
static void free_text(void)
{
    line * p = text;
    while(p) {
        line *next = p->next;
        free(p->content);
        free(p);
        p = next;
    }
    text = NULL;
}

// 扩展某一行的列数
static void expand_col(line * node,int expand)
{
    if(expand == 0){
        if(node){
            node->capacity+=EXPAND_NUM;
            node->content = (char *)realloc(node->content, node->capacity+1);
        }
    }else{
        if(node){
            node->capacity+=expand;
            node->content = (char *)realloc(node->content, node->capacity+1);
        }
    }
}

// 从文件读取内容到数组上
static void read_file(char *filename)
{
    // 检查文件是否存在
    if(access(filename, F_OK) != 0){    // 若文件不存在
        FILE *fp = fopen(filename,"w");            // 创建文件
        if (fp) {
            line_count = 1;
            insert_text(0, NULL);
            fclose(fp);
        } else {
            perror("fopen(w)");
        }
        return;
    }

    // 以只读方式打开文件
    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        perror("fopen(r)");
        return;
    }

    line_count = 0;
    char buffer[EXPAND_NUM] = {0};  // 用于读取文件一行内容
    
    // 逐行读取文件内容
    while(fgets(buffer, EXPAND_NUM, fp)!=NULL){
        size_t len = strlen(buffer);
        char *buf = (char *)calloc(EXPAND_NUM, sizeof(char));
        int length = EXPAND_NUM;
        strcpy(buf, buffer);

        // 处理超长行
        while(buffer[len-1] != '\n' && !feof(fp)){  // 如果没遇到 换行 或者没到文件末尾
            if(fgets(buffer,EXPAND_NUM,fp)!=NULL){
                len = strlen(buffer);
                length+=len;
                buf = (char *)realloc(buf, length);
                strcat(buf, buffer);   // 拼接
            }
        }
        buf[strcspn(buf, "\n")] = '\0'; // 去掉 '\n'
        // syslog(LOG_INFO,"buf:%s\n",buf);
        insert_text(line_count, buf);
        free(buf);
        line_count++;
    }
    if(line_count == 0){   // 若为空文本
        insert_text(0, NULL);
        line_count = 1;
    }
    fclose(fp); // 关闭文件描述符
}

// 将链表内容写入文件
static void write_file(char *filename)
{
    line *p = text;
    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        perror("fopen");
        return;
    }
    for(int i=0;i<line_count;++i){
        fprintf(fp,"%s\n",p->content);    // 也可以用fwrite
        p = p->next;
    }
    fclose(fp); // 关闭文件描述符
    // printf_line();
}

// 显示文本内容
static void display_text(void)
{
    line *p = text;
    // 清屏 - 直到refresh生效 
    clear();    // 代替方案 erase() 不会清除窗口的属性设置（如颜色）
    for(int i = 0; i < line_count; i++){    // 逐行打印文本
        // 在指定位置打印文本
        mvprintw(i, 0, "%s", p->content);   // 第i行第0列开始打印
        p = p->next;
    }
    move(current_row, current_col);        // 移动光标到当前行列
    refresh();  // 刷新窗口显示
    // printf_line();
}

// 插入字符到当前行
static void insert_char(char ch)
{
    if((ch<32 && ch!='\n') || ch>126 ) return;
    line * p = text;
    int index = current_row;
    while(index--) {
        p = p->next;
    }
    if(p == NULL)   return;

    if(ch == '\n'){
        line_count++;
        // 移动当前行的光标后的内容到新行
        int move_size = p->length - current_col;
        char buf[move_size+1];
        if(move_size > 0){
            memmove(buf, p->content + current_col, move_size);
            memset(p->content + current_col, 0, move_size);
            p->length = current_col;
        }
        buf[move_size] = '\0';
        insert_text(++current_row, buf);
        current_col = 0;
    }else{
        if(p->length >= p->capacity){   // 若需要扩展
            expand_col(p,0);
        }
        int move_size = p->length - current_col;
        memmove(p->content + current_col + 1, p->content + current_col, move_size);
        p->content[current_col] = ch;
        p->content[++p->length] = '\0';
        ++current_col;
    }
    display_text(); // 更新显示
}

// 删除字符
static void delete_char(void)
{
    line * p = text;
    line * q = NULL;
    int index = current_row;
    while(index--) {
        q = p;
        p = p->next;
    }
    if(p == NULL)   return;
    if(current_col > 0){
        int move_size = p->length - current_col;
        memmove(p->content + current_col - 1, p->content + current_col, move_size);
        p->content[--p->length] = '\0';
        current_col--;
    }else if(current_col==0 && current_row!=0){
        int len = strlen(p->content);
        if(len+q->length > q->capacity){
            expand_col(q, len-(q->capacity-q->length));
        }
        memmove(q->content + q->length,p->content,len+1);
        delete_text(current_row);
        q->length+=len;
        -- current_row;
        -- line_count;
        current_col = q->length;
    }
    display_text(); // 更新显示
}

// 处理用户输入
static void handle_input(void)
{
    // 输入逻辑
    int ch;
    while ((ch = getch()) != 0x1B) { // 输入 Esc键 退出
        switch (ch) {
            case KEY_UP: // 上箭头
            {
                if (current_row > 0){
                    line * p = text;
                    int index = current_row-1;
                    while(index--) {
                        p = p->next;
                    }
                    if(p == NULL)   break;
                    if(current_col > p->length) current_col=p->length;
                    current_row--;
                }
                break;
            }
            case KEY_DOWN: // 下箭头
            {
                if(current_row < line_count - 1){
                    line * p = text;
                    int index = current_row+1;
                    while(index--) {
                        p = p->next;
                    }
                    if(p == NULL)   break;
                    if(current_col > p->length) current_col=p->length;
                    current_row++;
                }
                break;
            }
            case KEY_LEFT: // 左箭头
                if(current_col > 0) current_col--;
                break;
            case KEY_RIGHT: // 右箭头
            {
                line * p = text;
                int index = current_row;
                while(index--) {
                    p = p->next;
                }
                if(p == NULL)   return;
                if(current_col < p->length) current_col++;
            }
                break;
            case KEY_BACKSPACE: // 后退
                delete_char();
                break;                
            default:    // 处理普通字符输入
                insert_char(ch);    // 插入字符
                break;
        }
        move(current_row, current_col);
        refresh();
    }
}

// 传递文件名
int main(int argc, char *argv[])
{
    // 拿到要编辑的文件名 - argv[1]为传递的文件名
    if(argc < 2){  // 传入参数不够
        fprintf(stderr, "缺少文件名参数\n");
        return EXIT_FAILURE;
    }
// 打开 syslog
openlog("ncurses_program", LOG_PID | LOG_CONS, LOG_USER);
    // 初始化 ncurses
    initscr();              // 自动检测终端类型，初始化标准窗口(stdscr)
    cbreak();               // 终端不再缓冲输入，输入字符立刻传递给程序，可CTRL+C中断
    noecho();               // 不在终端上显示输入的字符
    keypad(stdscr, TRUE);   // 键盘输入支持，允许使用箭头键等特殊键

    // 读取文件内容
    read_file(argv[1]);
    display_text(); // 显示文本内容

    handle_input(); // 处理键盘输入

    // 保存文件
    write_file(argv[1]);

    // 结束 ncurses
    endwin(); // 结束 ncurses 模式
    free_text();
closelog();
    return EXIT_SUCCESS;
}