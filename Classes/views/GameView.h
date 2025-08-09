#ifndef __GAME_VIEW_H__
#define __GAME_VIEW_H__

#include "cocos2d.h"
#include "../models/GameModel.h"
#include "../configs/models/LevelConfig.h"
#include "../configs/loaders/ConfigManager.h"
#include "CardView.h"
#include <vector>
#include <memory>
#include <map>
#include <functional>

USING_NS_CC;

/**
 * 游戏主视图
 * 负责管理整个游戏界面的显示，包括桌面牌区和手牌堆区域
 */
class GameView : public Layer {
public:
    /**
     * 创建游戏视图
     * @return 游戏视图实例
     */
    static GameView* create();
    
    /**
     * 初始化游戏视图
     * @return 是否初始化成功
     */
    virtual bool init() override;
    
    /**
     * 析构函数
     */
    virtual ~GameView();
    
    /**
     * 根据关卡配置初始化游戏布局
     * @param levelConfig 关卡配置
     * @param gameModel 游戏数据模型
     * @return 是否初始化成功
     */
    bool initWithLevelConfig(std::shared_ptr<LevelConfig> levelConfig, 
                            std::shared_ptr<GameModel> gameModel);
    
    /**
     * 获取指定ID的卡牌视图
     * @param cardId 卡牌ID
     * @return 卡牌视图，未找到返回nullptr
     */
    CardView* getCardView(int cardId) const;
    
    /**
     * 获取桌面牌区的所有卡牌视图
     * @return 桌面牌视图列表
     */
    const std::vector<CardView*>& getPlayfieldCardViews() const { return _playfieldCardViews; }
    
    /**
     * 获取手牌堆的所有卡牌视图
     * @return 手牌堆视图列表
     */
    const std::vector<CardView*>& getStackCardViews() const { return _stackCardViews; }
    
    /**
     * 获取当前底牌视图
     * @return 底牌视图
     */
    CardView* getCurrentCardView() const { return _currentCardView; }
    
    /**
     * 设置当前底牌视图
     * @param cardView 新的底牌视图
     */
    void setCurrentCardView(CardView* cardView) { _currentCardView = cardView; }
    
    /**
     * 获取当前底牌区域节点
     * @return 底牌区域节点
     */
    Node* getCurrentCardArea() const { return _currentCardArea; }
    
    /**
     * 获取手牌堆区域节点
     * @return 手牌堆区域节点
     */
    Node* getStackArea() const { return _stackArea; }
    
    /**
     * 获取桌面区域节点
     * @return 桌面区域节点
     */
    Node* getPlayfieldArea() const { return _playfieldArea; }
    
    /**
     * 获取卡牌ID到视图的映射
     * @return 卡牌视图映射
     */
    const std::map<int, CardView*>& getCardViewMap() const { return _cardViewMap; }
    
    /**
     * 设置卡牌点击回调
     * @param callback 点击回调函数
     */
    using CardClickCallback = std::function<void(CardView*, std::shared_ptr<CardModel>)>;
    void setCardClickCallback(const CardClickCallback& callback) { _cardClickCallback = callback; }
    
    /**
     * 获取卡牌点击回调
     * @return 点击回调函数
     */
    const CardClickCallback& getCardClickCallback() const { return _cardClickCallback; }

    /**
     * 设置回退按钮点击回调
     * @param callback 回退回调函数
     */
    using UndoCallback = std::function<void()>;
    void setUndoCallback(const UndoCallback& callback) { _undoCallback = callback; }
    
    /**
     * 播放卡牌移动动画
     * @param cardView 卡牌视图
     * @param targetPosition 目标位置
     * @param duration 动画时长
     * @param callback 完成回调
     */
    void playCardMoveAnimation(CardView* cardView, const Vec2& targetPosition, 
                              float duration = 0.3f, const std::function<void()>& callback = nullptr);
    
    /**
     * 更新游戏显示
     * @param gameModel 游戏数据模型
     */
    void updateDisplay(std::shared_ptr<GameModel> gameModel);
    
    /**
     * 清除所有卡牌视图
     */
    void clearAllCards();

protected:
    /**
     * 创建桌面牌区
     * @param levelConfig 关卡配置
     * @param gameModel 游戏数据模型
     */
    void createPlayfieldArea(std::shared_ptr<LevelConfig> levelConfig, 
                            std::shared_ptr<GameModel> gameModel);
    
    /**
     * 创建手牌堆区域
     * @param levelConfig 关卡配置
     * @param gameModel 游戏数据模型
     */
    void createStackArea(std::shared_ptr<LevelConfig> levelConfig, 
                        std::shared_ptr<GameModel> gameModel);
    
    /**
     * 创建底牌区域
     * @param gameModel 游戏数据模型
     */
    void createCurrentCardArea(std::shared_ptr<GameModel> gameModel);
    
    /**
     * 创建背景和UI元素
     * @param levelConfig 关卡配置
     */
    void createBackground(std::shared_ptr<LevelConfig> levelConfig);

    /**
     * 创建UI按钮
     */
    void createUIButtons();
    
    /**
     * 处理卡牌点击事件
     * @param cardView 被点击的卡牌视图
     * @param cardModel 卡牌数据模型
     */
    void onCardClicked(CardView* cardView, std::shared_ptr<CardModel> cardModel);

private:
    // 卡牌视图容器
    std::vector<CardView*> _playfieldCardViews;     // 桌面牌视图
    std::vector<CardView*> _stackCardViews;         // 手牌堆视图
    CardView* _currentCardView;                     // 当前底牌视图
    
    // 卡牌ID到视图的映射
    std::map<int, CardView*> _cardViewMap;
    
    // 区域节点
    Node* _playfieldArea;                           // 桌面牌区域
    Node* _stackArea;                               // 手牌堆区域
    Node* _currentCardArea;                         // 底牌区域
    
    // 回调函数
    CardClickCallback _cardClickCallback;
    UndoCallback _undoCallback;
    
    // UI元素
    MenuItemLabel* _undoButton;                     // 回退按钮
    
    // 配置管理器（不持有，只引用）
    ConfigManager* _configManager;
};

#endif // __GAME_VIEW_H__
