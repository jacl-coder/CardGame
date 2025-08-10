#ifndef __PLAYFIELD_CONTROLLER_H__
#define __PLAYFIELD_CONTROLLER_H__

#include "BaseController.h"
#include <memory>
#include <vector>
#include <functional>

USING_NS_CC;

// 前向声明
class GameView;

/**
 * 桌面牌控制器
 * 负责处理桌面牌区域的所有逻辑，包括卡牌点击、匹配判断、移动动画等
 * 按照README要求：处理卡片相关的具体逻辑，连接视图和模型
 */
class PlayFieldController : public BaseController {
public:
    /**
     * 卡牌点击回调
     * @param success 操作是否成功
     * @param cardModel 被点击的卡牌
     */
    using CardClickCallback = std::function<void(bool success, std::shared_ptr<CardModel> cardModel)>;
    
    /**
     * 构造函数
     */
    PlayFieldController();
    
    /**
     * 析构函数
     */
    virtual ~PlayFieldController();
    
    /**
     * 初始化控制器
     * @param gameModel 游戏数据模型
     * @param undoManager 撤销管理器
     * @return 是否初始化成功
     */
    bool init(std::shared_ptr<GameModel> gameModel, UndoManager* undoManager);
    
    /**
     * 初始化视图
     * @param playfieldCardViews 桌面牌视图列表
     * @return 是否初始化成功
     */
    bool initView(const std::vector<CardView*>& playfieldCardViews);
    
    /**
     * 处理卡牌点击事件
     * 按照README要求：检查卡片是否满足移动条件
     * @param cardId 被点击的卡牌ID
     * @param callback 处理完成回调
     * @return 是否开始处理
     */
    bool handleCardClick(int cardId, const CardClickCallback& callback = nullptr);
    
    /**
     * 执行桌面牌到底牌的替换操作
     * 按照README要求：replaceTrayWithPlayFieldCard执行
     * @param cardId 要移动的卡牌ID
     * @param callback 完成回调
     * @return 是否开始执行
     */
    bool replaceTrayWithPlayFieldCard(int cardId, const AnimationCallback& callback = nullptr);
    
    /**
     * 检查卡牌是否可以与当前底牌匹配
     * @param cardModel 要检查的卡牌
     * @return 是否可以匹配
     */
    bool canMatchWithCurrentCard(std::shared_ptr<CardModel> cardModel) const;
    
    /**
     * 获取所有可匹配的桌面牌
     * @return 可匹配的卡牌列表
     */
    std::vector<std::shared_ptr<CardModel>> getMatchableCards() const;
    
    /**
     * 高亮显示可匹配的卡牌
     * @param highlight 是否高亮
     */
    void highlightMatchableCards(bool highlight);
    
    /**
     * 设置卡牌点击回调
     * @param callback 回调函数
     */
    void setCardClickCallback(const CardClickCallback& callback) { _cardClickCallback = callback; }
    
    /**
     * 获取桌面牌视图
     * @param cardId 卡牌ID
     * @return 卡牌视图，未找到返回nullptr
     */
    CardView* getCardView(int cardId) const;
    
    /**
     * 注册新的卡牌视图（用于回退恢复等场景）
     * @param cardView 要注册的卡牌视图
     */
    void registerCardView(CardView* cardView);
    
    /**
     * 更新显示
     */
    void updateDisplay();
    
    /**
     * 提供当前底牌视图，用于执行收编为当前底牌显示视图的操作
     */
    void setCurrentCardView(CardView* current) { _currentCardView = current; }
    CardView* getCurrentCardView() const { return _currentCardView; }
    
    /**
     * 设置当前底牌区域节点，用于直接操作底牌区域
     */
    void setCurrentCardArea(Node* area) { _currentCardArea = area; }
    
    /**
     * 设置GameView引用，用于同步更新底牌视图
     */
    void setGameView(GameView* gameView) { _gameView = gameView; }

protected:
    /**
     * 检查卡牌移动条件
     * @param cardModel 要移动的卡牌
     * @return 是否满足条件
     */
    bool checkMoveConditions(std::shared_ptr<CardModel> cardModel) const;
    
    /**
     * 处理卡牌点击的内部逻辑
     * @param cardView 被点击的卡牌视图
     * @param cardModel 卡牌数据模型
     */
    void onCardClicked(CardView* cardView, std::shared_ptr<CardModel> cardModel);

private:
    // 视图组件
    std::vector<CardView*> _playfieldCardViews;     // 桌面牌视图列表
    std::map<int, CardView*> _cardViewMap;          // 卡牌ID到视图的映射

    // 回调函数
    CardClickCallback _cardClickCallback;           // 卡牌点击回调

    // 状态标志
    bool _isInitialized;                            // 是否已初始化
    bool _isProcessingClick;                        // 是否正在处理点击

    // 新增：当前底牌相关引用（不拥有，仅引用）
    CardView* _currentCardView = nullptr;
    Node* _currentCardArea = nullptr;
    GameView* _gameView = nullptr;
};

#endif // __PLAYFIELD_CONTROLLER_H__
