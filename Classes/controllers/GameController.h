#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include "cocos2d.h"
#include "../models/GameModel.h"
#include "../configs/models/LevelConfig.h"
#include "../configs/loaders/LevelConfigLoader.h"
#include "../services/GameModelFromLevelGenerator.h"
#include "../views/GameView.h"
#include "../views/CardView.h"
#include "../managers/UndoManager.h"
#include "PlayFieldController.h"
#include "StackController.h"
#include <memory>

USING_NS_CC;

/**
 * 游戏主控制器
 * 按照README要求：管理整个游戏流程，协调各子控制器
 * 不直接处理具体的业务逻辑，而是委托给专门的子控制器
 * 依赖多个Services和Managers
 */
class GameController {
public:
    /**
     * 构造函数
     */
    GameController();
    
    /**
     * 析构函数
     */
    virtual ~GameController();
    
    /**
     * 初始化游戏控制器
     * @param gameView 游戏视图
     * @return 是否初始化成功
     */
    bool init(GameView* gameView);
    
    /**
     * 开始游戏
     * 按照README要求的初始化流程：
     * 1. 加载LevelConfig
     * 2. 使用GameModelFromLevelGenerator生成GameModel
     * 3. 初始化各子控制器
     * 4. 创建GameView
     * 5. 初始化各子控制器的视图
     * @param levelId 关卡ID
     * @return 是否开始成功
     */
    bool startGame(int levelId);
    
    /**
     * 重新开始当前关卡
     * @return 是否重新开始成功
     */
    bool restartGame();
    
    /**
     * 暂停游戏
     */
    void pauseGame();
    
    /**
     * 恢复游戏
     */
    void resumeGame();
    
    /**
     * 获取当前游戏状态
     * @return 游戏状态
     */
    GameState getCurrentGameState() const;
    
    /**
     * 获取当前关卡ID
     * @return 关卡ID
     */
    int getCurrentLevelId() const;
    
    /**
     * 获取游戏数据模型
     * @return 游戏数据模型
     */
    std::shared_ptr<GameModel> getGameModel() const { return _gameModel; }
    
    /**
     * 获取关卡配置
     * @return 关卡配置
     */
    std::shared_ptr<LevelConfig> getLevelConfig() const { return _levelConfig; }

    /**
     * 获取桌面牌控制器
     * @return 桌面牌控制器
     */
    PlayFieldController* getPlayFieldController() const { return _playfieldController; }

    /**
     * 获取手牌堆控制器
     * @return 手牌堆控制器
     */
    StackController* getStackController() const { return _stackController; }

    /**
     * 获取撤销管理器
     * @return 撤销管理器
     */
    UndoManager* getUndoManager() const { return _undoManager; }

protected:
    /**
     * 初始化各子控制器
     * 按照README要求：PlayFieldController::init, StackController::init, UndoManager::init
     * @return 是否初始化成功
     */
    bool initializeSubControllers();

    /**
     * 初始化各子控制器的视图
     * 按照README要求：StackController::initView, PlayFieldController::initView
     * @return 是否初始化成功
     */
    bool initializeSubControllerViews();

    /**
     * 初始化游戏视图
     * @return 是否初始化成功
     */
    bool initializeGameView();

    /**
     * 处理桌面牌点击事件
     * @param success 操作是否成功
     * @param cardModel 被点击的卡牌
     */
    void onPlayFieldCardClicked(bool success, std::shared_ptr<CardModel> cardModel);

    /**
     * 处理手牌堆操作事件
     * @param success 操作是否成功
     * @param cardModel 操作的卡牌
     */
    void onStackOperationPerformed(bool success, std::shared_ptr<CardModel> cardModel);

    /**
     * 更新游戏显示
     */
    void updateGameDisplay();

    /**
     * 检查游戏胜利条件
     * @return 是否胜利
     */
    bool checkWinCondition();

    /**
     * 处理游戏胜利
     */
    void handleGameWin();

    /**
     * 处理游戏失败
     */
    void handleGameLose();

private:
    // 核心组件
    GameView* _gameView;                                // 游戏视图
    std::shared_ptr<GameModel> _gameModel;              // 游戏数据模型
    std::shared_ptr<LevelConfig> _levelConfig;          // 关卡配置
    LevelConfigLoader _configLoader;                    // 配置加载器

    // 子控制器（按照README要求）
    PlayFieldController* _playfieldController;          // 桌面牌控制器
    StackController* _stackController;                  // 手牌堆控制器
    UndoManager* _undoManager;                          // 撤销管理器

    // 游戏状态
    int _currentLevelId;                                // 当前关卡ID
    bool _isInitialized;                                // 是否已初始化
};

#endif // __GAME_CONTROLLER_H__
