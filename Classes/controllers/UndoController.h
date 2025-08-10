#ifndef __UNDO_CONTROLLER_H__
#define __UNDO_CONTROLLER_H__

#include "cocos2d.h"
#include "../models/GameModel.h"
#include "../models/UndoModel.h"
#include "../managers/UndoManager.h"
#include "../views/GameView.h"
#include "../views/CardView.h"
#include "PlayFieldController.h"
#include "StackController.h"
#include <memory>
#include <functional>

USING_NS_CC;

/**
 * 撤销控制器
 * 专门负责处理撤销相关的动画和视图恢复逻辑
 * 将原本在GameController中的undo视图操作代码迁移到这里，实现职责分离
 */
class UndoController {
public:
    /**
     * 构造函数
     */
    UndoController();
    
    /**
     * 析构函数
     */
    virtual ~UndoController();
    
    /**
     * 初始化撤销控制器
     * @param gameView 游戏视图
     * @param gameModel 游戏数据模型
     * @param undoManager 撤销管理器
     * @param playfieldController 桌面牌控制器
     * @param stackController 手牌堆控制器
     * @return 是否初始化成功
     */
    bool init(GameView* gameView, 
              std::shared_ptr<GameModel> gameModel,
              UndoManager* undoManager,
              PlayFieldController* playfieldController,
              StackController* stackController);
    
    /**
     * 执行撤销操作（包括数据恢复和动画）
     * @return 是否撤销成功
     */
    bool performUndo();

protected:
    /**
     * 执行撤销动画
     * @param undoModel 撤销操作模型
     */
    void performUndoAnimation(std::shared_ptr<UndoModel> undoModel);

    /**
     * 执行桌面牌撤销动画
     * @param undoModel 撤销操作模型
     */
    void performPlayfieldCardUndoAnimation(std::shared_ptr<UndoModel> undoModel);

    /**
     * 执行手牌堆撤销动画
     * @param undoModel 撤销操作模型
     */
    void performStackCardUndoAnimation(std::shared_ptr<UndoModel> undoModel);

    /**
     * 恢复卡牌到桌面区域
     * @param cardView 卡牌视图
     * @param cardModel 卡牌模型
     * @param absolutePos 绝对位置
     * @param originalZOrder 原始z序
     */
    void restoreCardToPlayfield(CardView* cardView, std::shared_ptr<CardModel> cardModel, const Vec2& absolutePos, int originalZOrder);

    /**
     * 恢复卡牌到手牌堆
     * @param cardView 卡牌视图
     * @param cardModel 卡牌模型
     * @param absolutePos 绝对位置
     */
    void restoreCardToStack(CardView* cardView, std::shared_ptr<CardModel> cardModel, const Vec2& absolutePos);

    /**
     * 更新底牌显示
     */
    void updateCurrentCardDisplay();

    /**
     * 更新游戏显示
     */
    void updateGameDisplay();

private:
    // 核心组件引用
    GameView* _gameView;                                // 游戏视图
    std::shared_ptr<GameModel> _gameModel;              // 游戏数据模型
    UndoManager* _undoManager;                          // 撤销管理器
    PlayFieldController* _playfieldController;          // 桌面牌控制器
    StackController* _stackController;                  // 手牌堆控制器
    
    // 状态
    bool _isInitialized;                                // 是否已初始化
};

#endif // __UNDO_CONTROLLER_H__
