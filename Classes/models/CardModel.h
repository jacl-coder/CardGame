#ifndef __CARD_MODEL_H__
#define __CARD_MODEL_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"

USING_NS_CC;

/**
 * 花色类型枚举
 */
enum CardSuitType {
    CST_NONE = -1,
    CST_CLUBS,      // 梅花
    CST_DIAMONDS,   // 方块
    CST_HEARTS,     // 红桃
    CST_SPADES,     // 黑桃
    CST_NUM_CARD_SUIT_TYPES
};

/**
 * 牌面类型枚举
 */
enum CardFaceType {
    CFT_NONE = -1,
    CFT_ACE,
    CFT_TWO,
    CFT_THREE,
    CFT_FOUR,
    CFT_FIVE,
    CFT_SIX,
    CFT_SEVEN,
    CFT_EIGHT,
    CFT_NINE,
    CFT_TEN,
    CFT_JACK,
    CFT_QUEEN,
    CFT_KING,
    CFT_NUM_CARD_FACE_TYPES
};

// 兼容性别名，用于平滑迁移
using CardSuitType_Legacy = CardSuitType;
using CardFaceType_Legacy = CardFaceType;

/**
 * 卡牌数据模型
 * 负责存储卡牌的基本信息，包括花色、点数、位置等
 */
class CardModel {
public:
    /**
     * 构造函数
     * @param face 牌面类型
     * @param suit 花色类型
     * @param position 卡牌位置
     */
    CardModel(CardFaceType face, CardSuitType suit, const Vec2& position = Vec2::ZERO);
    
    /**
     * 默认构造函数
     */
    CardModel();
    
    /**
     * 析构函数
     */
    virtual ~CardModel();
    
    // Getter方法
    CardFaceType getFace() const { return _face; }
    CardSuitType getSuit() const { return _suit; }
    Vec2 getPosition() const { return _position; }
    int getCardId() const { return _cardId; }
    bool isFlipped() const { return _isFlipped; }
    
    // Setter方法
    void setFace(CardFaceType face) { _face = face; }
    void setSuit(CardSuitType suit) { _suit = suit; }
    void setPosition(const Vec2& position) { _position = position; }
    void setCardId(int cardId) { _cardId = cardId; }
    void setFlipped(bool flipped) { _isFlipped = flipped; }
    
    /**
     * 获取卡牌的数值（用于匹配计算）
     * @return 卡牌数值（A=1, 2=2, ..., K=13）
     */
    int getCardValue() const;
    
    /**
     * 检查两张卡牌是否可以匹配
     * @param other 另一张卡牌
     * @return 是否可以匹配
     */
    bool canMatchWith(const CardModel& other) const;
    
    /**
     * 获取卡牌的字符串表示
     * @return 卡牌字符串（如"♠A", "♥K"）
     */
    std::string toString() const;
    
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
    CardFaceType _face;         // 牌面类型
    CardSuitType _suit;         // 花色类型
    Vec2 _position;             // 卡牌位置
    int _cardId;                // 卡牌唯一ID
    bool _isFlipped;            // 是否翻开
    
    static int s_nextCardId;    // 下一个卡牌ID
    
    /**
     * 生成下一个卡牌ID
     * @return 新的卡牌ID
     */
    int generateCardId();
    
    /**
     * 获取花色符号
     * @param suit 花色类型
     * @return 花色符号字符串
     */
    std::string getSuitSymbol(CardSuitType suit) const;
    
    /**
     * 获取牌面符号
     * @param face 牌面类型
     * @return 牌面符号字符串
     */
    std::string getFaceSymbol(CardFaceType face) const;
};

#endif // __CARD_MODEL_H__
