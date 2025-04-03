# Linux 知识学习与工具开发

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/yourusername/reponame.svg?style=social)](https://github.com/yourusername/reponame/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/yourusername/reponame.svg?style=social)](https://github.com/yourusername/reponame/network/members)

本项目包含我对 Linux 系统知识的理解以及一些实用工具的开发和实现。

## 📌 目录
- [项目简介](#-项目简介)
- [功能特性](#-功能特性)
- [当前项目结构](#-当前项目结构)
- [快速开始](#-快速开始)
- [贡献指南](#-贡献指南)
- [许可证](#-许可证)
- [联系方式](#-联系方式)

## 🚀 项目简介

这个仓库主要用于：
- 记录和整理 Linux 相关知识
- 开发实用的 Linux 小工具
- 分享 Linux 系统使用经验

## ✨ 功能特性

### 文件复制工具 (file_copier.ct)
✅ 支持基本文件复制功能  
✅ 包含进度显示功能  
✅ 支持大文件分块处理  
✅ 包含错误处理机制  


### Linux 应用层学习 (linux_application_layer)
📚 第一阶段学习内容包括：
- 文件IO操作
- 进程管理
- 线程编程
- 网络编程基础


## 📚 当前项目结构

- `linux_application_layer/` - Linux 应用层学习
  - `phase1/` - 第一阶段内容
    - `file_copier.c` - 文件复制器
    - `simple_editor.c` - 文件编辑器


## 📥 快速开始

### 文件复制器使用

```bash
# linux下使用示例
# 编译
gcc file_copier.c -o copy
# 执行
./copy source_file destination_file
```

### 文件编辑器使用

```bash
# linux下使用示例
# 编译
gcc simple_editor.c -o edit -lncurses
# 执行(后面加上文件名称，如：1.txt)
./edit file
# 可以在终端输入字母、数字及符号（暂且不支持中文输入），按Esc退出保存
```


## 🤝 贡献指南

我们欢迎任何形式的贡献！请遵循以下流程：

- Fork 本项目
  - 创建你的分支 (git checkout -b feature/你的功能)
  - 提交你的修改 (git commit -m '添加了很棒的功能')
  - 推送到分支 (git push origin feature/你的功能)
  - 创建 Pull Request

### 代码规范
- 遵循 Linux 内核代码风格
- 使用有意义的变量名
- 添加必要的注释
- 保持函数短小精悍


## 📜 许可证

本项目采用 MIT 许可证 - 详情请参阅 LICENSE 文件。


## 📞 联系方式
如有任何问题或建议，欢迎联系：

邮箱: 3257136093@qq.com 
博客: [个人技术博客](https://blog.csdn.net/m0_64657422?spm=1010.2135.3001.10640)
