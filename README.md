# 360智能行车辅助系统服务器

## 项目简介
本项目是360智能行车辅助系统的服务器端实现，包含前置服务器和后置服务器两个主要组件。系统采用C++开发，实现了高性能的网络通信、数据处理和存储功能。

## 系统架构
系统分为两个主要部分：

### 前置服务器 (Intelligent_driving_assistance_system)
- 负责与客户端进行实时通信
- 实现TCP服务器功能
- 处理心跳检测
- 管理连接池和线程池
- 使用Epoll实现高并发

### 后置服务器 (backServer)
- 负责数据处理和存储
- 实现数据库操作
- 处理图像数据
- 管理任务队列
- 实现进程间通信(IPC)

## 主要功能
- 实时数据传输
- 图像处理和分析
- 数据库存储和查询
- 多线程任务处理
- 心跳检测机制
- 进程间通信

## 技术特点
- 使用C++开发，保证高性能
- 采用Epoll实现高并发
- 实现线程池管理
- 使用SQLite进行数据存储
- 支持TCP/IP通信
- 模块化设计，易于扩展

## 系统要求
- 操作系统：Linux
- 编译器：支持C++11或更高版本
- 依赖库：
  - SQLite3
  - 标准C++库

## 安装说明
1. 克隆项目到本地
```bash
git clone [项目地址]
```

2. 编译项目
```bash
# 使用Visual Studio打开解决方案文件
Intelligent_driving_assistance_system.sln
```

3. 配置数据库
- 确保SQLite3已正确安装
- 检查数据库连接配置

## 使用方法
1. 启动后置服务器
```bash
cd backServer/bin
./backServer
```

2. 启动前置服务器
```bash
cd Intelligent_driving_assistance_system/bin
./Intelligent_driving_assistance_system
```

## 项目结构
```
├── Intelligent_driving_assistance_system/  # 前置服务器
│   ├── CEpollServer.cpp/h                 # Epoll服务器实现
│   ├── CTcpServer.cpp/h                   # TCP服务器实现
│   ├── CThreadPool.cpp/h                  # 线程池实现
│   └── ...
├── backServer/                            # 后置服务器
│   ├── DBBusiness.cpp/h                   # 数据库业务逻辑
│   ├── CImage.cpp/h                       # 图像处理
│   ├── IPC.cpp/h                          # 进程间通信
│   └── ...
└── Intelligent_driving_assistance_system.sln  # 解决方案文件
```

## 开发团队
- 360智能行车辅助系统开发团队

## 许可证
[待定]

## 联系方式
[待补充] 
