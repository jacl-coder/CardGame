#ifndef __LEVEL_CONFIG_H__
#define __LEVEL_CONFIG_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"
#include "../../models/CardModel.h"
#include <vector>
#include <memory>

USING_NS_CC;

/**
 * 卡牌配置数据结构
 * 用于存储关卡配置文件中的单张卡牌信息
 */
struct CardConfigData {
    CardFaceType cardFace;      // 牌面类型
    CardSuitType cardSuit;      // 花色类型
    Vec2 position;              // 位置坐标
    
    /**
     * 构造函数
     */
    CardConfigData()
        : cardFace(CFT_ACE)
        , cardSuit(CST_CLUBS)
        , position(Vec2::ZERO) {
    }
    
    CardConfigData(CardFaceType face, CardSuitType suit, const Vec2& pos)
        : cardFace(face)
        , cardSuit(suit)
        , position(pos) {
    }
    
    /**
     * 序列化到JSON
     */
    rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 从JSON反序列化
     */
    void fromJson(const rapidjson::Value& json);
};

/**
 * 关卡配置类
 * 负责存储和管理单个关卡的配置数据
 * 包括桌面牌区(Playfield)和手牌堆(Stack)的配置
 */
class LevelConfig {
public:
    /**
     * 构造函数
     */
    LevelConfig();
    
    /**
     * 析构函数
     */
    virtual ~LevelConfig();
    
    // 基本属性
    int getLevelId() const { return _levelId; }
    void setLevelId(int levelId) { _levelId = levelId; }
    
    std::string getLevelName() const { return _levelName; }
    void setLevelName(const std::string& name) { _levelName = name; }
    
    // 桌面牌区配置
    const std::vector<CardConfigData>& getPlayfieldCards() const { return _playfieldCards; }
    void addPlayfieldCard(const CardConfigData& cardData);
    void clearPlayfieldCards();
    
    // 手牌堆配置
    const std::vector<CardConfigData>& getStackCards() const { return _stackCards; }
    void addStackCard(const CardConfigData& cardData);
    void clearStackCards();
    
    // 游戏区域尺寸配置
    Size getPlayfieldSize() const { return _playfieldSize; }
    void setPlayfieldSize(const Size& size) { _playfieldSize = size; }
    
    Size getStackSize() const { return _stackSize; }
    void setStackSize(const Size& size) { _stackSize = size; }
    
    /**
     * 验证配置数据的有效性
     * @return 配置是否有效
     */
    bool isValid() const;
    
    /**
     * 获取配置摘要信息
     * @return 配置摘要字符串
     */
    std::string getSummary() const;
    
    /**
     * 序列化到JSON
     * @return JSON对象
     */
    rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 从JSON反序列化
     * @param json JSON对象
     * @return 是否成功
     */
    bool fromJson(const rapidjson::Value& json);
    
    /**
     * 重置配置数据
     */
    void reset();

private:
    int _levelId;                                   // 关卡ID
    std::string _levelName;                         // 关卡名称
    std::vector<CardConfigData> _playfieldCards;    // 桌面牌区卡牌配置
    std::vector<CardConfigData> _stackCards;        // 手牌堆卡牌配置
    Size _playfieldSize;                            // 主牌区尺寸 (1080*1500)
    Size _stackSize;                                // 堆牌区尺寸 (1080*580)
    
    /**
     * 序列化卡牌配置数组
     */
    rapidjson::Value serializeCardArray(const std::vector<CardConfigData>& cards,
                                       rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 反序列化卡牌配置数组
     */
    std::vector<CardConfigData> deserializeCardArray(const rapidjson::Value& jsonArray) const;
};

#endif // __LEVEL_CONFIG_H__
