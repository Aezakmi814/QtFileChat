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

## 环境搭建与配置

### 环境要求

| 依赖 | 版本 | 说明 |
|------|------|------|
| Qt | 5.14.2 (MinGW 7.3.0 64-bit) | 开发框架 |
| MySQL | 5.7+ | 数据库（存储用户、好友关系） |
| 编译器 | g++ 7.3.0 | MinGW 自带 |

### 第一步：数据库配置

#### 1. 安装 MySQL

确保你的机器上已安装 MySQL 5.7+，并启动 MySQL 服务。

#### 2. 创建数据库和表

```sql
CREATE DATABASE test;
USE test;

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

#### 3. 修改数据库连接信息

找到 `Server/operatedb.cpp:14-18`，修改为你的 MySQL 实际配置：

```cpp
// 第14行附近，修改为你的数据库信息
m_db.setHostName("localhost");   // MySQL 主机地址
m_db.setPort(3306);              // MySQL 端口（默认3306）
m_db.setUserName("root");        // 你的 MySQL 用户名
m_db.setPassword("123456");      // 你的 MySQL 密码 ← 改成你自己的密码！
m_db.setDatabaseName("test");    // 数据库名（与上面 CREATE DATABASE 一致）
```

> ⚠️ **安全提醒**：当前数据库密码是硬编码在源码中的。如果你计划将代码开源或分享，建议后续将数据库配置抽离到独立配置文件（如 `db.config`）并加入 `.gitignore`，避免密码泄露。

### 第二步：配置文件

服务器和客户端各有一个 `connect.config` 文件，格式相同：

```
IP地址
端口号
文件存储根目录
```

#### 服务器配置：`Server/connect.config`

```
127.0.0.1      ← 服务器监听 IP（默认本机）
5000           ← 端口号
./filesys      ← 用户文件存储根目录（相对路径，会在 Server 同目录下创建）
```

#### 客户端配置：`client/connect.config`

```
127.0.0.1      ← 服务器 IP（如果服务器在其他机器，改成对应的 IP）
5000           ← 端口号（必须与服务器一致）
./filesys      ← 文件下载时的本地根目录
```

> 如果你把服务器部署在远程机器上，只需要改客户端配置文件的第一行 IP 即可。

### 第三步：编译运行

```bash
# 编译服务器
cd Server
qmake Server.pro
mingw32-make -j4

# 编译客户端
cd client
qmake client.pro
mingw32-make -j4

# 运行（先服务端，后客户端）
# 启动 Server/debug/Server.exe
# 启动 client/debug/client.exe
```

> 首次启动服务器后，会在 `Server/filesys/` 目录下自动为注册用户创建个人文件夹。

### 常见问题

| 问题 | 解决方法 |
|------|----------|
| 服务器启动提示"数据库连接失败" | 检查 `operatedb.cpp` 中的 MySQL 用户名/密码/端口是否正确 |
| 客户端连接不上服务器 | 检查客户端 `connect.config` 中的 IP 和端口是否和服务器一致 |
| 注册成功但无法登录 | 检查 MySQL 的 `user_info` 表是否创建成功 |
| 上传文件失败 | 检查服务器 `connect.config` 中的文件根目录是否有写入权限 |

---

> **💡 提示**：如果你想修改文件存储路径，改 `Server/connect.config` 第三行即可；
> 如果想修改数据库，改 `Server/operatedb.cpp` 第14~18行的连接参数；
> 如果要改端口/IP，两边的 `connect.config` 都需同步修改。

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
