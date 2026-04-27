# Poker Multiplayer Engine

**实时多人德州扑克引擎 | Scalable Real-time Poker Server | 德州俱乐部源码 | 德州私人局 | 德州朋友局**

一个高性能、可扩展的**服务端权威（Server-Authoritative）**多人扑克游戏后端框架，专为实时同步和高并发环境设计。采用现代 C++ + WebSocket 架构，适合构建德州扑克俱乐部、私人局、朋友局、联盟系统等应用。

**GitHub Stars**：⭐ 持续更新中 | **License**：MIT

---

## ✨ 核心特性

- **服务端权威架构**：所有游戏逻辑（发牌、牌型计算、动作验证）均在服务端执行，客户端仅发送操作指令，确保公平性和防作弊基础。
- **实时状态同步**：基于 WebSocket 的低延迟事件驱动同步，支持多人同桌实时对战。
- **房间与会话管理**：支持公开桌、私人局、朋友局、俱乐部房间，可灵活配置桌型（9人/6人/短牌等）。
- **匹配系统**：智能 matchmaking，支持快速加入、好友邀请、私密房间。
- **AI 机器人引擎**：内置可配置的 AI Bot，用于测试、填充桌子或单机练习模式。
- **事件驱动游戏流程**：清晰的牌局生命周期（PreFlop → Flop → Turn → River → Showdown），易于扩展其他扑克变体。
- **高并发设计**：支持线程安全、连接池、房间隔离，单机可承载较高并发（结合 Redis 等可水平扩展）。
- **模块化架构**：核心逻辑、网络层、数据库操作、日志系统完全解耦，便于二次开发。
- **日志与数据持久化**：支持手牌历史记录、玩家行为日志入库，为反作弊和数据分析提供基础。

---

## 🛠 技术栈

- **后端语言**：C++（现代 C++ 标准，推荐 C++17/20）
- **网络层**：WebSocket（高效实时通信，支持二进制协议）
- **架构模式**：Server-Authoritative + Event-Driven + Room-Based
- **数据库支持**：MySQL / PostgreSQL（通过 DBOperator 层），推荐搭配 Redis 做缓存与 Pub/Sub
- **其他**：多线程安全（ThreadLockManager）、协议定义（TARS 等）、日志系统
- **客户端**：不包含 UI（推荐搭配 Unity / React / Flutter 等任意前端），仅需实现 WebSocket 通信协议即可接入

---

## 🚀 快速开始


git clone https://github.com/pokerdeveloper/poker-multiplayer-engine.git
cd poker-multiplayer-engine
# 根据你的环境编译（推荐使用 CMake）
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
./poker_server
使用 Docker（推荐，无需编译）

# 1. 拉取镜像并运行（示例镜像地址需您实际提供）
docker run -p 8080:8080 pokerdev/poker-engine:latest

# 2. 使用 WebSocket 测试工具连接 ws://localhost:8080

## 📈 本项目实现的技术优化

**性能与扩展性**
引入 Redis 缓存 + Pub/Sub 实现分布式房间管理和跨服同步
使用 Kubernetes + Docker 容器化部署，支持自动扩缩容
优化牌型计算与洗牌算法（使用加密安全随机数源）

**反作弊与安全**
服务端严格验证所有动作（下注、弃牌、超时等）
增加行为分析模块（异常下注模式、胜率偏差检测）
客户端完整性校验 + 设备指纹 + 风控系统
手牌历史智能审计

**功能扩展**
支持多种扑克变体（短牌、奥马哈、Pineapple 等）
增加 SNG、MTT 锦标赛模式、赏金赛、联盟系统
社交系统：好友、聊天、表情、礼物、实时语音（WebRTC）
多语言支持（简/繁中 + 英文 + 韩文+马来西亚语+泰语+日语等）

**客户端与前端**
提供 Unity + Lua 示例客户端（或 React + WebSocket H5 版）
优化 UI/UX，提升新手引导与留存

**数据与运维**
接入完整埋点与分析系统（玩家留存、流失、付费转化）
自动化 CI/CD + 监控告警（Prometheus + Grafana）
压力测试工具与基准数据

**合规与 monetization（（
金币/钻石系统（非真实货币模式）
支付接口抽象层（便于接入当地支付渠道）
年龄分级、隐私政策支持



## 📊 当前状态与路线图
当前版本：基础核心引擎可用，适合学习、原型开发和二次开发。
完整商用级德州俱乐部解决方案，支持高并发、强反作弊、多模式赛事，并提供一键部署脚本与详细文档。
欢迎社区贡献 Pull Request，一起完善这个开源扑克引擎！

## 📞 联系与合作

Telegram：@alibabama401
Email：ttpoker733@gmail.com

欢迎讨论技术实现、生产部署、定制开发等专业话题。

## 产品部分截图
![1房间列表](https://github.com/user-attachments/assets/55b7e4e6-875b-471f-bfe4-ebc01ec79e61)
![1管理 - 副本](https://github.com/user-attachments/assets/2662b77e-1a07-4fe0-b7c6-5e8349d5ab32)
![1管理俱乐部币 - 副本](https://github.com/user-attachments/assets/add2e1c3-efc2-4eb6-8578-acdf0f3d4c5c)
![4房间2-6人桌](https://github.com/user-attachments/assets/e2645541-12ce-4945-83a0-7e421787a90b)
![4房间2-9人桌](https://github.com/user-attachments/assets/8409e9a6-baf4-4c97-87ff-eabaea198461)
![AB7AF159B3F73F5AEB2C720E957074F4](https://github.com/user-attachments/assets/a9e778df-0c5e-48cb-85b1-a7cab553755d)
![6EF906300DBB3B2C29583B8027D40F5C](https://github.com/user-attachments/assets/2c46c343-68f9-4b7f-a06d-50d1abe1bb70)
## 🧠 Architecture Overview

This project follows a server-authoritative architecture:

- All game logic runs on the server
- Clients only send actions
- State synchronization handled in real-time

Designed for:
- High concurrency environments
- Scalable multiplayer systems
- Real-time game consistency
## ⚠️ Disclaimer | 免责声明 | 免責聲明

- This project does NOT support real-money gambling  
  本项目不支持任何真实货币赌博  
  本專案不支持任何真實貨幣賭博  

- No payment or financial systems included  
  不包含任何支付或金融系统  
  不包含任何支付或金融系統  

- For educational use only  
  仅用于学习与研究用途  
  僅用於學習與研究用途  

Any misuse is not the responsibility of the author.  
任何滥用行为与作者无关。  
任何濫用行為與作者無關。  

本项目仅供学习、研究和技术演示使用，不支持也不鼓励用于真实货币赌博。任何商业用途请自行确保符合当地法律法规，作者不承担相关责任。




