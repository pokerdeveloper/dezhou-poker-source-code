# 🃏 德州扑克源码 | 德州俱乐部源码 | 德州扑克服务器引擎

**德州源码 · 德州联盟源码 · 德州私人局 · 德州朋友局源码**  
实时多人德州扑克高并发服务器引擎（C++17/20 + WebSocket）

**Texas Hold'em Poker Multiplayer Source Code** | **Real-time Multiplayer Poker Server Engine** | Scalable Game Backend

[![Stars](https://img.shields.io/github/stars/pokerdeveloper/poker-multiplayer-engine.svg)](https://github.com/pokerdeveloper/poker-multiplayer-engine/stargazers)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

## ✨ 项目亮点（核心特性）

- **服务器权威架构**：所有游戏逻辑由服务器处理，防作弊、防外挂，保证公平性
- **高并发实时同步**：基于 WebSocket + 事件驱动，支持高并发多人对战
- **完整德州扑克逻辑**：支持经典德州（9人/6人桌）、私人局、朋友局、俱乐部模式
- **AI Bot 支持**：内置智能机器人，可用于测试或填充桌位
- **技术栈**：C++17/20 + TARS（或你的框架） + MySQL/Redis + WebSocket，代码结构清晰、易扩展
- **跨平台友好**：服务器端独立，可搭配 Unity / Web / React 等任意客户端使用

这是一个**面向开发者**的开源德州扑克多人游戏引擎，适合学习高并发游戏服务器架构，或快速搭建德州俱乐部、私人局系统。

> **仅供学习和研究使用，请严格遵守当地法律法规，不得用于任何真实货币赌博。**

---

## 🚀 快速开始

**```bash**
git clone https://github.com/pokerdeveloper/poker-multiplayer-engine.git
cd poker-multiplayer-engine
# 按照 docs/build.md 中的 cmake 编译步骤执行
Docker 一键运行（推荐新手）：
docker run -p 8080:8080 pokerdev/poker-engine:latest
# 使用 WebSocket 测试工具连接 ws://localhost:8080
详细编译、配置、协议说明和部署指南请查看 docs

## 📸 项目截图

<img width="1176" height="2400" alt="84E607B21587F105FEC5D0FF009746ED" src="https://github.com/user-attachments/assets/8f89dcea-c902-4d13-abe6-8125ec4ccce0" />
<img width="720" height="1280" alt="B578474D3E2935735437109224188836" src="https://github.com/user-attachments/assets/5d28b05a-a226-41e6-a596-d6e358a31d3d" />
<img width="720" height="1280" alt="03178711F04324F5D4A10564B9708E18" src="https://github.com/user-attachments/assets/fed4c2fd-7220-4160-8181-128859d81358" />
<img width="720" height="1280" alt="374AE612FE1D4C01FD716C3D6E3753C8" src="https://github.com/user-attachments/assets/01070961-89c8-4aaa-b4d1-e9a3404589b0" />
<img width="720" height="1280" alt="19F73728400F466D1B26B09E04E062FC" src="https://github.com/user-attachments/assets/d33bdb47-59f4-4bdf-8e11-36d69c464b8d" />
<img width="720" height="1280" alt="8ABCFEB5516352EE96F77272B39FCF45" src="https://github.com/user-attachments/assets/58eadff9-7acc-4f3f-9e05-f16226f97476" />
<img width="720" height="1280" alt="5E679337A03783B8AF3653C018F1D959" src="https://github.com/user-attachments/assets/97f75126-ae12-4e2b-aa06-3fce277751f7" />
<img width="720" height="1280" alt="3A54764D72CEBB70A5A80965A93C3ACC" src="https://github.com/user-attachments/assets/58ea2c79-ae16-45b9-82ab-f299dc611025" />
<img width="720" height="1280" alt="0FC579ED11E86A65C3FE6024EC105B8B" src="https://github.com/user-attachments/assets/9bb7d6db-52f6-4746-aa7a-bb996661b3a2" />


## 📈 未来路线图（Roadmap）

支持更多扑克变体（奥马哈、短牌进阶、Pineapple 等）
增加 SNG、MTT 锦标赛模式和联盟系统
完善分布式部署（Kubernetes + Redis 集群）
提供 Unity / Web 示例客户端代码
增强反作弊模块（行为分析、设备指纹等）
多语言支持与国际化

欢迎社区通过 Issue 和 Pull Request 共同完善！

## 📞 联系与合作

Telegram：@alibabama401
Email：ttpoker733@gmail.com

欢迎讨论技术实现、部署问题、功能定制或合作开发等专业话题。

## ⭐ 支持项目
如果你觉得这个项目对你有帮助，欢迎 Star 支持一下！
也欢迎 Fork 后进行二次开发，并提交 Pull Request 贡献代码。

再次声明：本项目为开源学习项目，不包含任何支付系统，也不支持真实货币赌博。任何滥用行为与作者无关。

关键词：德州扑克源码、德州源码、德州俱乐部源码、德州联盟源码、德州私人局源码、德州朋友局源码、Texas Hold'em source code、poker server、multiplayer poker engine、C++ poker game
