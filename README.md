# CardGame

一个基于 Cocos2d-x 引擎开发的卡牌游戏项目，采用现代化的 MVC 架构设计，支持配置驱动的关卡系统和完整的撤销功能。

## 📋 目录

- [项目简介](#项目简介)
- [特性](#特性)
- [技术栈](#技术栈)
- [项目结构](#项目结构)
- [快速开始](#快速开始)
- [架构设计](#架构设计)
- [配置系统](#配置系统)
- [开发指南](#开发指南)
- [贡献指南](#贡献指南)
- [程序设计文档](#程序设计文档)

## 🎮 项目简介

CardGame 是一个现代化的卡牌游戏项目，专注于提供良好的代码架构和可扩展性。项目采用分层设计，通过 JSON 配置文件驱动游戏内容，支持完整的撤销系统，便于快速开发和维护。

## ✨ 特性

- 🏗️ **模块化架构** - 采用 MVC 模式，各层职责明确
- ⚙️ **配置驱动** - 关卡内容通过 JSON 文件配置，支持热更新
- ↩️ **完整撤销系统** - 基于命令模式的撤销功能
- 🎯 **高可扩展性** - 易于添加新卡牌类型和游戏规则
- 📱 **跨平台支持** - 基于 Cocos2d-x，支持多平台部署
- 🎨 **现代化 UI** - 流畅的动画效果和用户体验

## 🛠️ 技术栈

- **游戏引擎**: Cocos2d-x
- **开发语言**: C++
- **构建工具**: Visual Studio 2022
- **配置格式**: JSON
- **架构模式**: MVC + 配置驱动

## 📁 项目结构

```
CardGame/
├── Classes/                    # 核心代码
│   ├── models/                # 数据模型层
│   │   ├── CardModel.*        # 卡牌数据模型
│   │   ├── GameModel.*        # 游戏状态模型
│   │   └── UndoModel.*        # 撤销操作模型
│   ├── views/                 # 视图表现层
│   │   ├── CardView.*         # 卡牌视图组件
│   │   └── GameView.*         # 游戏主视图
│   ├── controllers/           # 控制逻辑层
│   │   ├── BaseController.*   # 控制器基类
│   │   ├── GameController.*   # 游戏主控制器
│   │   ├── PlayFieldController.*  # 桌面牌控制器
│   │   └── StackController.*  # 手牌堆控制器
│   ├── managers/              # 管理器层
│   │   ├── ConfigManager.*    # 配置管理器
│   │   └── UndoManager.*      # 撤销管理器
│   ├── services/              # 业务服务层
│   │   └── GameModelFromLevelGenerator.*
│   ├── configs/               # 配置系统
│   │   ├── models/            # 配置数据模型
│   │   └── loaders/           # 配置加载器
│   └── utils/                 # 工具类
├── Resources/                  # 资源文件
│   ├── configs/               # 配置文件
│   │   └── data/
│   │       ├── levels/        # 关卡配置
│   │       └── ui/            # 界面配置
│   ├── res/                   # 图片资源
│   └── fonts/                 # 字体文件
├── cocos2d/                    # Cocos2d-x 引擎源码
└── proj.win32/                # Windows 项目文件
```

## 🚀 快速开始

### 环境要求

- Visual Studio 2022
- Cocos2d-x 引擎
- Windows 10/11

### 构建步骤

1. **克隆项目**
   ```bash
   git clone <repository-url>
   cd CardGame
   ```

2. **打开项目**
   ```bash
   # 使用 Visual Studio 2022 打开
   start proj.win32/CardGame.sln
   ```

3. **构建运行**
   - 选择 Debug/Release 配置
   - 按 F5 运行项目

### 配置关卡

编辑 `Resources/configs/data/levels/level_1.json` 来自定义关卡：

```json
{
  "LevelId": 1,
  "LevelName": "Level 1",
  "Playfield": [
    {
      "CardFace": 0,
      "CardSuit": 3,
      "Position": { "x": 250, "y": 1000 }
    }
  ],
  "Stack": [
    {
      "CardFace": 12,
      "CardSuit": 2,
      "Position": { "x": 0, "y": 0 }
    }
  ]
}
```

## 🏗️ 架构设计

### 设计原则

- **单一职责原则** - 每个类只负责一个明确的功能
- **依赖注入** - 通过构造函数和初始化方法注入依赖
- **配置驱动** - 游戏规则和布局通过 JSON 配置文件控制
- **事件驱动** - 使用回调函数实现组件间解耦

### 核心架构

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Controllers   │    │     Views       │    │     Models      │
│                 │    │                 │    │                 │
│ • GameController│────│ • GameView      │────│ • GameModel     │
│ • PlayFieldCtrl │    │ • CardView      │    │ • CardModel     │
│ • StackCtrl     │    │                 │    │ • UndoModel     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                        ┌─────────────────┐
                        │    Managers     │
                        │                 │
                        │ • ConfigManager │
                        │ • UndoManager   │
                        └─────────────────┘
```

### 数据流

1. **配置加载**: ConfigManager 加载 JSON 配置文件
2. **模型初始化**: GameModelFromLevelGenerator 根据配置生成 GameModel
3. **视图创建**: GameView 根据 GameModel 创建视图组件
4. **用户交互**: Controllers 处理用户输入，更新 Models
5. **视图更新**: Views 监听 Models 变化，自动刷新显示

## ⚙️ 配置系统

### 配置文件类型

| 文件 | 用途 | 位置 |
|------|------|------|
| `level_*.json` | 关卡配置 | `Resources/configs/data/levels/` |
| `layout_config.json` | UI布局配置 | `Resources/configs/data/ui/` |
| `animation_config.json` | 动画配置 | `Resources/configs/data/ui/` |

### 配置热更新

配置文件支持运行时重新加载，无需重启游戏即可看到配置变更效果。

## 📚 开发指南

### 添加新关卡

1. 在 `Resources/configs/data/levels/` 创建新的 JSON 文件
2. 按照现有格式配置卡牌布局
3. 在游戏中调用 `GameController::startGame(levelId)` 加载关卡

### 扩展卡牌类型

1. 在 `CardModel.h` 中扩展枚举类型
2. 更新 `CardView` 的渲染逻辑
3. 在游戏规则中添加匹配逻辑
4. 更新配置文件格式

### 自定义控制器

继承 `BaseController` 类，实现特定的游戏逻辑：

```cpp
class CustomController : public BaseController {
public:
    bool init(/* parameters */) override;
    void handleCustomLogic();
};
```

## 🔧 代码规范

### 命名约定

- **类名**: PascalCase (`CardModel`, `GameController`)
- **函数名**: camelCase (`handleCardClick`, `updateDisplay`)
- **成员变量**: 下划线前缀 (`_cardModel`, `_isInitialized`)
- **常量**: k前缀 (`kMaxUndoSteps`, `kDefaultFontSize`)

### 文件组织

- 头文件(.h)和实现文件(.cpp)分离
- 按功能模块组织目录结构
- 每个类一个文件，文件名与类名一致

## 🤝 贡献指南

1. **Fork** 项目到你的 GitHub 账户
2. **创建** 功能分支 (`git checkout -b feature/AmazingFeature`)
3. **提交** 你的修改 (`git commit -m 'Add some AmazingFeature'`)
4. **推送** 到分支 (`git push origin feature/AmazingFeature`)
5. **创建** Pull Request

### 提交规范

- 使用清晰的提交信息
- 确保代码通过所有测试
- 遵循项目的代码规范
- 更新相关文档

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 📞 联系方式

如果你有任何问题或建议，欢迎：

- 创建 [Issue](../../issues)
- 发起 [Pull Request](../../pulls)
- 发送邮件至 [laix1024@gmail.com]

## 📖 程序设计文档

如需了解详细的技术架构、扩展方法和代码实现，请参阅：

**[DESIGN.md](DESIGN.md)** - 程序设计文档

该文档包含：
- 🏗️ **详细架构设计** - 完整的系统架构和设计模式说明
- 🔧 **扩展开发指南** - 如何添加新卡牌类型和撤销功能的步骤说明
- 📋 **代码规范** - 统一的编码标准和最佳实践
- ⚡ **性能优化** - 针对性的优化建议和实现方法

---

⭐ 如果这个项目对你有帮助，请给它一个星标！