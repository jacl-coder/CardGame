#ifndef __CARD_LAYOUT_CONFIG_H__
#define __CARD_LAYOUT_CONFIG_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"

USING_NS_CC;

/**
 * 卡牌布局配置类
 * 负责管理卡牌内部元素的布局配置，包括数字、花色的位置等
 * 替代硬编码的卡牌布局常量，提供可配置的卡牌布局系统
 */
class CardLayoutConfig {
public:
    /**
     * 位置配置结构（使用相对坐标，0.0-1.0）
     */
    struct RelativePosition {
        float x, y;
        
        RelativePosition() : x(0.5f), y(0.5f) {}
        RelativePosition(float posX, float posY) : x(posX), y(posY) {}
        
        Vec2 toAbsolutePosition(const Size& cardSize) const {
            return Vec2(cardSize.width * x, cardSize.height * y);
        }
    };
    
    /**
     * 构造函数
     */
    CardLayoutConfig();
    
    /**
     * 析构函数
     */
    virtual ~CardLayoutConfig();
    
    // 卡牌元素位置配置
    RelativePosition getBigNumberPosition() const { return _bigNumberPosition; }
    void setBigNumberPosition(const RelativePosition& position) { _bigNumberPosition = position; }
    
    RelativePosition getSmallNumberPosition() const { return _smallNumberPosition; }
    void setSmallNumberPosition(const RelativePosition& position) { _smallNumberPosition = position; }
    
    RelativePosition getSuitPosition() const { return _suitPosition; }
    void setSuitPosition(const RelativePosition& position) { _suitPosition = position; }
    
    RelativePosition getCardBackTextPosition() const { return _cardBackTextPosition; }
    void setCardBackTextPosition(const RelativePosition& position) { _cardBackTextPosition = position; }
    
    // 便捷方法：获取绝对位置
    Vec2 getBigNumberAbsolutePosition(const Size& cardSize) const {
        return _bigNumberPosition.toAbsolutePosition(cardSize);
    }
    
    Vec2 getSmallNumberAbsolutePosition(const Size& cardSize) const {
        return _smallNumberPosition.toAbsolutePosition(cardSize);
    }
    
    Vec2 getSuitAbsolutePosition(const Size& cardSize) const {
        return _suitPosition.toAbsolutePosition(cardSize);
    }
    
    Vec2 getCardBackTextAbsolutePosition(const Size& cardSize) const {
        return _cardBackTextPosition.toAbsolutePosition(cardSize);
    }
    
    /**
     * 从JSON加载配置
     * @param json JSON对象
     * @return 是否加载成功
     */
    bool fromJson(const rapidjson::Value& json);
    
    /**
     * 序列化到JSON
     * @param allocator JSON分配器
     * @return JSON对象
     */
    rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 验证配置有效性
     * @return 是否有效
     */
    bool isValid() const;
    
    /**
     * 重置为默认配置
     */
    void resetToDefault();
    
    /**
     * 获取配置摘要
     * @return 摘要字符串
     */
    std::string getSummary() const;

private:
    RelativePosition _bigNumberPosition;        // 大数字位置（卡牌中心）
    RelativePosition _smallNumberPosition;      // 小数字位置（左上角）
    RelativePosition _suitPosition;             // 花色位置（右上角）
    RelativePosition _cardBackTextPosition;     // 卡牌背面文字位置
    
    /**
     * 解析相对位置从JSON
     */
    RelativePosition parseRelativePositionFromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化相对位置到JSON
     */
    rapidjson::Value serializeRelativePositionToJson(const RelativePosition& position, rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 验证相对位置有效性
     */
    bool isValidRelativePosition(const RelativePosition& position) const;
};

#endif // __CARD_LAYOUT_CONFIG_H__
