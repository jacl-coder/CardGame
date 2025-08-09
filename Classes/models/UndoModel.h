#ifndef __UNDO_MODEL_H__
#define __UNDO_MODEL_H__

#include "cocos2d.h"
#include "CardModel.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"
#include <memory>

USING_NS_CC;

/**
 * 撤销操作类型枚举
 */
enum class UndoOperationType {
    NONE = -1,              // 无操作
    CARD_MOVE,              // 卡牌移动（桌面牌到底牌）
    CARD_FLIP,              // 卡牌翻转
    STACK_OPERATION         // 手牌堆操作（手牌堆到底牌）
};

/**
 * 撤销操作数据模型
 * 记录单次操作的详细信息，用于回退功能
 */
class UndoModel {
public:
    /**
     * 构造函数
     * @param operationType 操作类型
     */
    UndoModel(UndoOperationType operationType);
    
    /**
     * 析构函数
     */
    virtual ~UndoModel();
    
    // 基本属性
    UndoOperationType getOperationType() const { return _operationType; }
    void setOperationType(UndoOperationType type) { _operationType = type; }
    
    // 卡牌相关
    std::shared_ptr<CardModel> getSourceCard() const { return _sourceCard; }
    void setSourceCard(std::shared_ptr<CardModel> card) { _sourceCard = card; }
    
    std::shared_ptr<CardModel> getTargetCard() const { return _targetCard; }
    void setTargetCard(std::shared_ptr<CardModel> card) { _targetCard = card; }
    
    // 位置/层级相关
    Vec2 getSourcePosition() const { return _sourcePosition; }
    void setSourcePosition(const Vec2& position) { _sourcePosition = position; }
    
    Vec2 getTargetPosition() const { return _targetPosition; }
    void setTargetPosition(const Vec2& position) { _targetPosition = position; }

    int getSourceZOrder() const { return _sourceZOrder; }
    void setSourceZOrder(int z) { _sourceZOrder = z; }
    
    // 状态相关
    bool getSourceFlippedState() const { return _sourceFlippedState; }
    void setSourceFlippedState(bool flipped) { _sourceFlippedState = flipped; }
    
    bool getTargetFlippedState() const { return _targetFlippedState; }
    void setTargetFlippedState(bool flipped) { _targetFlippedState = flipped; }
    
    // 分数相关
    int getScoreDelta() const { return _scoreDelta; }
    void setScoreDelta(int delta) { _scoreDelta = delta; }
    
    // 时间戳
    long long getTimestamp() const { return _timestamp; }
    void setTimestamp(long long timestamp) { _timestamp = timestamp; }
    
    /**
     * 创建手牌堆到底牌的撤销记录
     * @param sourceCard 源卡牌（手牌堆顶部）
     * @param targetCard 目标卡牌（原底牌）
     * @param sourcePos 源位置
     * @param targetPos 目标位置
     * @param scoreDelta 分数变化
     * @return 撤销记录
     */
    static std::shared_ptr<UndoModel> createStackToCurrentAction(
        std::shared_ptr<CardModel> sourceCard,
        std::shared_ptr<CardModel> targetCard,
        const Vec2& sourcePos,
        const Vec2& targetPos,
        int scoreDelta = 0
    );
    
    /**
     * 创建桌面牌到底牌的撤销记录
     * @param sourceCard 源卡牌（桌面牌）
     * @param targetCard 目标卡牌（原底牌）
     * @param sourcePos 源位置
     * @param targetPos 目标位置
     * @param scoreDelta 分数变化
     * @return 撤销记录
     */
    static std::shared_ptr<UndoModel> createPlayfieldToCurrentAction(
        std::shared_ptr<CardModel> sourceCard,
        std::shared_ptr<CardModel> targetCard,
        const Vec2& sourcePos,
        const Vec2& targetPos,
        int scoreDelta = 0,
        int sourceZOrder = 0
    );
    
    /**
     * 创建翻牌操作的撤销记录
     * @param card 被翻的卡牌
     * @param originalFlippedState 原始翻牌状态
     * @return 撤销记录
     */
    static std::shared_ptr<UndoModel> createFlipCardAction(
        std::shared_ptr<CardModel> card,
        bool originalFlippedState
    );
    
    /**
     * 获取操作描述
     * @return 操作描述字符串
     */
    std::string getActionDescription() const;

    /**
     * 获取操作摘要
     * @return 操作摘要字符串
     */
    std::string getOperationSummary() const { return getActionDescription(); }
    
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
    UndoOperationType _operationType;           // 操作类型
    std::shared_ptr<CardModel> _sourceCard;     // 源卡牌
    std::shared_ptr<CardModel> _targetCard;     // 目标卡牌
    Vec2 _sourcePosition;                       // 源位置
    Vec2 _targetPosition;                       // 目标位置
    int _sourceZOrder;                          // 源卡牌原始z序
    bool _sourceFlippedState;                   // 源卡牌翻牌状态
    bool _targetFlippedState;                   // 目标卡牌翻牌状态
    int _scoreDelta;                            // 分数变化
    long long _timestamp;                       // 时间戳
    
    /**
     * 获取当前时间戳
     * @return 时间戳
     */
    long long getCurrentTimestamp() const;
};

#endif // __UNDO_MODEL_H__
