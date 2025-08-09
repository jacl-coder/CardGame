## 项目说明（程序设计文档）

本工程是一个使用 Cocos2d-x 的纸牌游戏，仓库包含引擎源码 `cocos2d/` 与 Windows 工程 `proj.win32/`，可在 VS2022 下开箱即用。

### 架构概览
- **模型层（Models）**：`Classes/models/`
  - `CardModel`：描述卡牌（花色、点数、位置、翻面、ID 等）。
  - `GameModel`：描述整体游戏状态（桌面牌、手牌堆、底牌、分数、移动次数、关卡等），并提供撤销操作的实际应用方法：`undoCardMove`、`undoCardFlip`、`undoStackOperation`。
  - `UndoModel`：记录一次可撤销操作的数据；`UndoOperationType` 枚举当前支持 `CARD_MOVE`/`CARD_FLIP`/`STACK_OPERATION`。
- **控制层（Controllers）**：`Classes/controllers/`
  - `GameController`：总控，负责关卡加载、模型生成、子控制器/视图初始化与整体流程（含 `performUndo`）。
  - `PlayFieldController`：桌面牌逻辑与交互（点击、匹配、动画、撤销记录）。
  - `StackController`：手牌堆逻辑与交互（翻牌、替换底牌、动画、撤销记录）。
- **视图层（Views）**：`Classes/views/`
  - `GameView`：主视图，持有各区域节点与 `CardView` 列表，转发点击/回退等交互。
  - `CardView`：单张卡牌的渲染、触摸与动画，绑定一个 `CardModel`。
- **配置与生成（Configs/Services）**：`Classes/configs` 与 `Classes/services`
  - `ConfigManager` 统一加载字体、UI、规则、布局、显示配置；`GameModelFromLevelGenerator` 根据关卡配置生成初始 `GameModel`。
- **管理器（Managers）**：
  - `UndoManager`：维护撤销栈，提供 `recordUndo`、`performUndo`、`canUndo`、`getUndoSummary` 等。

数据流与依赖关系简述：`GameController` 协调 `GameView` 与各控制器；控制器读写 `GameModel` 并通过 `UndoManager` 记录/执行撤销；`CardView` 负责具体渲染与动画；`ConfigManager` 提供全局配置。

### 如何新增一张卡牌（资源与显示）
1. 资源准备（`Resources/res/`）：
   - 数字贴图：`number/` 下新增所需牌面图片（若使用现有即无需新增）。
   - 花色贴图：`suits/` 下若新增花色，需添加对应 PNG。
2. 模型更新（如新增花色或点数）：
   - 在 `CardModel.h` 的 `CardSuitType`/`CardFaceType` 中添加枚举值，并在 `getSuitSymbol`/`getFaceSymbol`、`getCardValue`、`canMatchWith` 中补充处理逻辑（若规则有变化）。
3. 关卡/布局配置：
   - 在 `Resources/configs/data/levels/` 或相关配置中，加入该卡牌的初始位置/归属（桌面、手牌堆）。
   - 若需 UI 布局调整，修改 `Resources/configs/data/ui/*.json`。
4. 生成与显示：
   - `GameModelFromLevelGenerator` 根据关卡配置创建对应的 `CardModel` 并放入 `GameModel`。
   - `GameView` 初始化时会为每个 `CardModel` 生成 `CardView` 并注册点击，`CardView::updateDisplay`/`updateCardLayout` 会根据资源自动布局。

最小化新增场景：仅新增一张普通卡到关卡中，一般无需改代码，只需改关卡配置即可；如新增“花色/点数类型”，才需要按第 2 步扩展 `CardModel`。

### 如何新增一种“回退（Undo）”类型
目标：在现有 `CARD_MOVE`/`CARD_FLIP`/`STACK_OPERATION` 之外新增一种回退。例如“交换两张桌面牌”（示例）。

步骤：
1. 定义新类型：
   - 在 `UndoModel.h` 的 `UndoOperationType` 中新增枚举值，如 `SWAP_PLAYFIELD`。
2. 扩展 UndoModel 数据：
   - 视需求在 `UndoModel` 中新增字段（例如第二张卡的引用、原始位置/状态等），并在 `toJson`/`fromJson` 序列化中补充。
   - 提供静态建造方法，例如 `createSwapPlayfieldAction(cardA, cardB, posA, posB)`。
3. 记录撤销：
   - 在触发该操作的控制器里（如 `PlayFieldController`），完成业务动作时调用 `UndoManager::recordUndo(...)` 记录上述 `UndoModel`。
4. 应用撤销：
   - 在 `GameModel` 中新增处理函数（或复用一个分发）：实现 `bool undoSwapPlayfield(std::shared_ptr<UndoModel>)`，执行模型层面的状态复原（位置/翻面/分数/计数等）。
   - 在 `UndoManager::applyUndoToGameModel` 的分发中，增加对新类型的分支，调用 `gameModel->undoSwapPlayfield(...)`。
5. 视图复原（可选动画）：
   - 在 `GameController` 中的撤销动画分发（如 `performPlayfieldCardUndoAnimation`）里，为新类型添加相应的视图恢复逻辑，调用 `GameView`/`CardView` 播放动画并最终与模型一致。

验收建议：
- 单元测试或日志：使用 `UndoManager::getUndoSummary()` 打印最近操作摘要，确认记录与回退一致。
- 手动测试：先执行新操作，再点击回退按钮，观察模型与视图一致性。

### 代码规范与扩展建议
- 模型与视图分离：所有状态变更先落在 `GameModel`，视图仅渲染与动画。
- 撤销的三要素：源/目标卡、位置信息、可选的 z 序与翻面状态、分数变动、时间戳。
- 控制器职责单一：桌面牌逻辑进 `PlayFieldController`，手牌堆逻辑进 `StackController`，统一协调通过 `GameController`。
- 配置驱动：优先通过 JSON 配置新增内容，减少改动代码。

### Windows（VS2022）构建
仓库仅保留 `proj.win32/`，引擎随仓库提供，直接打开 `proj.win32/CardGame.sln` 构建运行。

