#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

#include "cocos2d.h"
#include "CardModel.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"
#include <vector>
#include <memory>

USING_NS_CC;

/**
 * 游戏状态枚举
 */
enum class GameState {
    INITIALIZING,   // 初始化中
    PLAYING,        // 游戏中
    PAUSED,         // 暂停
    GAME_OVER,      // 游戏结束
    WIN             // 胜利
};

/**
 * 游戏数据模型
 * 负责管理游戏的整体状态，包括所有卡牌、游戏区域、分数等
 */
class GameModel {
public:
    /**
     * 构造函数
     */
    GameModel();
    
    /**
     * 析构函数
     */
    virtual ~GameModel();
    
    // 游戏状态管理
    GameState getGameState() const { return _gameState; }
    void setGameState(GameState state) { _gameState = state; }
    
    // 桌面牌区管理
    const std::vector<std::shared_ptr<CardModel>>& getPlayfieldCards() const { return _playfieldCards; }
    void addPlayfieldCard(std::shared_ptr<CardModel> card);
    void removePlayfieldCard(int cardId);
    std::shared_ptr<CardModel> getPlayfieldCard(int cardId) const;
    void clearPlayfieldCards();
    
    // 手牌堆管理
    const std::vector<std::shared_ptr<CardModel>>& getStackCards() const { return _stackCards; }
    void addStackCard(std::shared_ptr<CardModel> card);
    std::shared_ptr<CardModel> removeTopStackCard();
    std::shared_ptr<CardModel> getTopStackCard() const;
    void clearStackCards();
    bool isStackEmpty() const { return _stackCards.empty(); }
    
    // 底牌管理
    std::shared_ptr<CardModel> getCurrentCard() const { return _currentCard; }
    void setCurrentCard(std::shared_ptr<CardModel> card) { _currentCard = card; }
    
    // 游戏统计
    int getScore() const { return _score; }
    void setScore(int score) { _score = score; }
    void addScore(int points) { _score += points; }
    
    int getMoveCount() const { return _moveCount; }
    void incrementMoveCount() { _moveCount++; }
    void setMoveCount(int count) { _moveCount = count; }
    
    // 关卡信息
    int getCurrentLevel() const { return _currentLevel; }
    void setCurrentLevel(int level) { _currentLevel = level; }
    
    // 游戏逻辑辅助方法
    /**
     * 检查是否有可匹配的卡牌
     * @return 是否有可匹配的卡牌
     */
    bool hasMatchableCards() const;
    
    /**
     * 获取所有可匹配的卡牌
     * @return 可匹配的卡牌列表
     */
    std::vector<std::shared_ptr<CardModel>> getMatchableCards() const;
    
    /**
     * 检查游戏是否胜利
     * @return 是否胜利
     */
    bool isGameWon() const;
    
    /**
     * 重置游戏状态
     */
    void resetGame();

    /**
     * 撤销卡牌移动操作
     * @param undoModel 撤销操作数据
     * @return 是否撤销成功
     */
    bool undoCardMove(std::shared_ptr<class UndoModel> undoModel);

    /**
     * 撤销卡牌翻转操作
     * @param undoModel 撤销操作数据
     * @return 是否撤销成功
     */
    bool undoCardFlip(std::shared_ptr<class UndoModel> undoModel);

    /**
     * 撤销手牌堆操作
     * @param undoModel 撤销操作数据
     * @return 是否撤销成功
     */
    bool undoStackOperation(std::shared_ptr<class UndoModel> undoModel);
    
    /**
     * 序列化到JSON
     * @return JSON对象
     */
    rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 从JSON反序列化
     * @param json JSON对象
     */
    void fromJson(const rapidjson::Value& json);

private:
    GameState _gameState;                                           // 游戏状态
    std::vector<std::shared_ptr<CardModel>> _playfieldCards;       // 桌面牌区卡牌
    std::vector<std::shared_ptr<CardModel>> _stackCards;           // 手牌堆卡牌
    std::shared_ptr<CardModel> _currentCard;                       // 当前底牌
    
    int _score;                                                     // 当前分数
    int _moveCount;                                                 // 移动次数
    int _currentLevel;                                              // 当前关卡
    
    /**
     * 序列化卡牌数组到JSON
     * @param cards 卡牌数组
     * @param allocator JSON分配器
     * @return JSON数组
     */
    rapidjson::Value serializeCards(const std::vector<std::shared_ptr<CardModel>>& cards,
                                   rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 从JSON反序列化卡牌数组
     * @param jsonArray JSON数组
     * @return 卡牌数组
     */
    std::vector<std::shared_ptr<CardModel>> deserializeCards(const rapidjson::Value& jsonArray) const;
};

#endif // __GAME_MODEL_H__
