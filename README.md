# QtFileChat — C/S 架构网络文件管理系统

基于 **Qt 5.14.2 (C++, MinGW)** 开发的客户端-服务器架构网络应用，集文件管理、即时通讯、好友社交于一体。

## 架构

```
┌──────────────────────────┐     TCP      ┌──────────────────────────┐
│   Qt 桌面客户端 (Client)  │◄───────────►│    Qt 服务器端 (Server)   │
│                          │   自定义 PDU  │                          │
│  ┌────────┐ ┌────────┐  │              │  ┌──────────┐ ┌────────┐ │
│  │ 好友管理 │ │ 文件管理 │  │              │  │ MyTcpServer│ │OperateDB│ │
│  │ 即时聊天 │ │ 上传下载 │  │              │  │ (线程池)  │ │ (MySQL) │ │
│  └────────┘ └────────┘  │              │  └──────────┘ └────────┘ │
└──────────────────────────┘              └──────────────────────────┘
```

## 功能

| 模块 | 功能 |
|------|------|
| 👤 用户系统 | 注册、登录、查找用户、在线用户列表 |
| 👥 好友系统 | 添加/删除好友、好友请求转发与确认、好友列表刷新 |
| 💬 即时聊天 | 在线好友间实时消息收发（服务器转发） |
| 📁 文件管理 | 创建/删除/重命名/移动文件和目录、目录树导航 |
| ⬆️ 文件上传 | 单线程/多线程两种模式、分块传输 |
| ⬇️ 文件下载 | 4096字节分块下载 |
 | 📤 文件分享 | 多选好友、确认接收、服务端文件复制 |

## 技术亮点

- **自定义二进制协议 (PDU)** — C 结构体 + 柔性数组实现变长消息，支持 20+ 消息类型
- **TCP 粘包/半包处理** — `QByteArray` 缓冲区 + 协议长度字段循环解析
- **线程池服务器** — 重写 `QTcpServer::incomingConnection()`，结合 `QThreadPool` + `QRunnable` 处理并发
- **单例模式** — Server、Client、OperateDB 全部使用线程安全单例
- **多线程上传** — `QThread` + `moveToThread` + `Qt::QueuedConnection` 跨线程安全通信
- **数据库持久化** — Qt SQL 模块 + MySQL，封装 OperateDB 单例

## 快速开始

### 环境要求

- Qt 5.14.2 (MinGW 7.3.0 64-bit)
- MySQL 5.7+

### 数据库初始化

```sql
CREATE DATABASE test;
CREATE TABLE user_info (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(32) UNIQUE,
    pwd VARCHAR(32),
    online INT DEFAULT 0
);
CREATE TABLE friend (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT,
    friend_id INT
);
```

### 编译运行

```bash
# 编译服务器
cd Server
qmake Server.pro
mingw32-make -j4

# 编译客户端
cd client
qmake client.pro
mingw32-make -j4

# 先启动 Server.exe，再启动 client.exe
```

### 配置

`connect.config` 格式（IP / 端口 / 文件根目录）：
```
127.0.0.1
5000
./filesys
```

## 协议设计

PDU (Protocol Data Unit) 结构：

```cpp
struct PDU {
    uint uiTotalLen;    // 协议总长度
    uint uiMsgLen;      // 消息体长度
    uint uiType;        // 消息类型枚举
    char caData[64];    // 固定参数区
    char caMsg[];       // 柔性数组（变长消息体）
};
```

支持消息类型：注册/登录、查找用户、在线用户、添加/删除好友、聊天、文件 CRUD、上传/下载/分享。

## 项目结构

```
QtFileChat/
├── Server/                # 服务器端
│   ├── server.h/cpp       # 服务器配置与启动
│   ├── mytcpserver.h/cpp  # TCP 服务器（线程池）
│   ├── mytcpsocket.h/cpp  # Socket 封装 + 粘包处理
│   ├── clienttask.h/cpp   # 线程池任务
│   ├── msghandler.h/cpp   # 消息业务逻辑
│   ├── operatedb.h/cpp    # MySQL 数据库操作
│   ├── protocol.h/cpp     # 协议定义
│   └── main.cpp
│
├── client/                # 桌面客户端
│   ├── client.h/cpp       # 主窗口 + 网络通信
│   ├── index.h/cpp        # 主界面（好友/文件页切换）
│   ├── friend.h/cpp       # 好友列表
│   ├── chat.h/cpp         # 聊天窗口
│   ├── onlineuser.h/cpp   # 在线用户
│   ├── file.h/cpp         # 文件管理器
│   ├── uploader.h/cpp     # 多线程上传器
│   ├── sharefile.h/cpp    # 文件分享
│   ├── reshandler.h/cpp   # 响应分发处理
│   ├── protocol.h/cpp     # 协议定义
│   └── main.cpp
│
├── .gitignore
└── README.md
```

## License

MIT
