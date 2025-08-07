#ifndef __STACK_CONTROLLER_H__
#define __STACK_CONTROLLER_H__

#include "cocos2d.h"
#include "../models/GameModel.h"
#include "../models/CardModel.h"
#include "../models/UndoModel.h"
#include "../views/CardView.h"
#include "../managers/UndoManager.h"
#include "../configs/loaders/ConfigManager.h"
#include <memory>
#include <vector>
#include <functional>

USING_NS_CC;

/**
 * 手牌堆控制器
 * 负责处理手牌堆区域的所有逻辑，包括翻牌、替换底牌等操作
 * 按照README要求：处理手牌堆相关的具体逻辑，连接视图和模型
 */
class StackController {
public:
    /**
     * 手牌操作回调
     * @param success 操作是否成功
     * @param cardModel 操作的卡牌
     */
    using StackOperationCallback = std::function<void(bool success, std::shared_ptr<CardModel> cardModel)>;
    
    /**
     * 动画完成回调
     * @param success 动画是否成功完成
     */
    using AnimationCallback = std::function<void(bool success)>;
    
    /**
     * 构造函数
     */
    StackController();
    
    /**
     * 析构函数
     */
    virtual ~StackController();
    
    /**
     * 初始化控制器
     * @param gameModel 游戏数据模型
     * @param undoManager 撤销管理器
     * @return 是否初始化成功
     */
    bool init(std::shared_ptr<GameModel> gameModel, UndoManager* undoManager);
    
    /**
     * 初始化视图
     * @param stackCardViews 手牌堆视图列表
     * @param currentCardView 当前底牌视图
     * @return 是否初始化成功
     */
    bool initView(const std::vector<CardView*>& stackCardViews, CardView* currentCardView);
    
    /**
     * 处理手牌堆顶部卡牌点击
     * 按照README需求1：手牌区翻牌替换
     * @param callback 处理完成回调
     * @return 是否开始处理
     */
    bool handleTopCardClick(const StackOperationCallback& callback = nullptr);
    
    /**
     * 执行手牌堆到底牌的替换操作
     * @param callback 完成回调
     * @return 是否开始执行
     */
    bool replaceCurrentWithTopCard(const AnimationCallback& callback = nullptr);
    
    /**
     * 翻开下一张手牌
     * @return 是否成功
     */
    bool revealNextCard();
    
    /**
     * 检查是否还有可用的手牌
     * @return 是否有可用手牌
     */
    bool hasAvailableCards() const;
    
    /**
     * 获取当前顶部卡牌
     * @return 顶部卡牌，无卡牌时返回nullptr
     */
    std::shared_ptr<CardModel> getTopCard() const;
    
    /**
     * 获取顶部卡牌视图
     * @return 顶部卡牌视图
     */
    CardView* getTopCardView() const;
    
    /**
     * 设置手牌操作回调
     * @param callback 回调函数
     */
    void setStackOperationCallback(const StackOperationCallback& callback) { _stackOperationCallback = callback; }
    
    /**
     * 更新手牌堆显示
     */
    void updateStackDisplay();
    
    /**
     * 更新底牌显示
     */
    void updateCurrentCardDisplay();

protected:
    /**
     * 记录撤销操作
     * @param sourceCard 源卡牌（手牌堆顶部）
     * @param targetCard 目标卡牌（原底牌）
     * @return 是否记录成功
     */
    bool recordUndoOperation(std::shared_ptr<CardModel> sourceCard, 
                            std::shared_ptr<CardModel> targetCard);
    
    /**
     * 执行卡牌移动动画
     * @param sourceView 源卡牌视图
     * @param targetPosition 目标位置
     * @param callback 完成回调
     */
    void playMoveAnimation(CardView* sourceView, const Vec2& targetPosition, 
                          const AnimationCallback& callback);
    
    /**
     * 处理手牌点击的内部逻辑
     * @param cardView 被点击的卡牌视图
     * @param cardModel 卡牌数据模型
     */
    void onStackCardClicked(CardView* cardView, std::shared_ptr<CardModel> cardModel);
    
    /**
     * 更新手牌堆的可见性和交互性
     */
    void updateStackInteractivity();

private:
    // 核心组件
    std::shared_ptr<GameModel> _gameModel;          // 游戏数据模型
    UndoManager* _undoManager;                      // 撤销管理器
    ConfigManager* _configManager;                  // 配置管理器

    // 视图组件
    std::vector<CardView*> _stackCardViews;         // 手牌堆视图列表
    CardView* _currentCardView;                     // 当前底牌视图
    std::map<int, CardView*> _cardViewMap;          // 卡牌ID到视图的映射

    // 回调函数
    StackOperationCallback _stackOperationCallback; // 手牌操作回调

    // 状态标志
    bool _isInitialized;                            // 是否已初始化
    bool _isProcessingOperation;                    // 是否正在处理操作
};

#endif // __STACK_CONTROLLER_H__
