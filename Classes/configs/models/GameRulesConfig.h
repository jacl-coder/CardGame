#ifndef __GAME_RULES_CONFIG_H__
#define __GAME_RULES_CONFIG_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"

USING_NS_CC;

/**
 * 游戏规则配置类
 * 负责管理游戏规则相关的配置，包括撤销设置、卡牌生成、匹配规则等
 * 替代硬编码的游戏规则常量，提供可配置的游戏规则系统
 */
class GameRulesConfig {
public:
    /**
     * 撤销设置结构
     */
    struct UndoSettings {
        int maxUndoSteps;       // 最大撤销步数
        bool enableUndo;        // 是否启用撤销
        
        UndoSettings() : maxUndoSteps(10), enableUndo(true) {}
        UndoSettings(int maxSteps, bool enable) : maxUndoSteps(maxSteps), enableUndo(enable) {}
    };
    
    /**
     * 卡牌生成设置结构
     */
    struct CardGenerationSettings {
        int startingCardId;     // 起始卡牌ID
        bool shuffleOnLoad;     // 加载时是否打乱
        
        CardGenerationSettings() : startingCardId(1000), shuffleOnLoad(false) {}
        CardGenerationSettings(int startId, bool shuffle) : startingCardId(startId), shuffleOnLoad(shuffle) {}
    };
    
    /**
     * 匹配规则设置结构
     */
    struct MatchingRules {
        bool allowCyclicMatching;   // 是否允许循环匹配（A和K）
        bool ignoreSuit;            // 是否忽略花色
        int matchDifference;        // 匹配的点数差值
        
        MatchingRules() : allowCyclicMatching(true), ignoreSuit(true), matchDifference(1) {}
        MatchingRules(bool cyclic, bool ignoreS, int diff) 
            : allowCyclicMatching(cyclic), ignoreSuit(ignoreS), matchDifference(diff) {}
    };
    
    /**
     * 构造函数
     */
    GameRulesConfig();
    
    /**
     * 析构函数
     */
    virtual ~GameRulesConfig();
    
    // 撤销设置
    UndoSettings getUndoSettings() const { return _undoSettings; }
    void setUndoSettings(const UndoSettings& settings) { _undoSettings = settings; }
    
    // 卡牌生成设置
    CardGenerationSettings getCardGenerationSettings() const { return _cardGenerationSettings; }
    void setCardGenerationSettings(const CardGenerationSettings& settings) { _cardGenerationSettings = settings; }
    
    // 匹配规则设置
    MatchingRules getMatchingRules() const { return _matchingRules; }
    void setMatchingRules(const MatchingRules& rules) { _matchingRules = rules; }
    
    // 便捷访问方法
    int getMaxUndoSteps() const { return _undoSettings.maxUndoSteps; }
    bool isUndoEnabled() const { return _undoSettings.enableUndo; }
    int getStartingCardId() const { return _cardGenerationSettings.startingCardId; }
    bool shouldShuffleOnLoad() const { return _cardGenerationSettings.shuffleOnLoad; }
    bool allowsCyclicMatching() const { return _matchingRules.allowCyclicMatching; }
    bool ignoresSuit() const { return _matchingRules.ignoreSuit; }
    int getMatchDifference() const { return _matchingRules.matchDifference; }
    
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
    UndoSettings _undoSettings;                     // 撤销设置
    CardGenerationSettings _cardGenerationSettings; // 卡牌生成设置
    MatchingRules _matchingRules;                   // 匹配规则设置
    
    /**
     * 解析撤销设置从JSON
     */
    UndoSettings parseUndoSettingsFromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化撤销设置到JSON
     */
    rapidjson::Value serializeUndoSettingsToJson(const UndoSettings& settings, rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 解析卡牌生成设置从JSON
     */
    CardGenerationSettings parseCardGenerationSettingsFromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化卡牌生成设置到JSON
     */
    rapidjson::Value serializeCardGenerationSettingsToJson(const CardGenerationSettings& settings, rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 解析匹配规则从JSON
     */
    MatchingRules parseMatchingRulesFromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化匹配规则到JSON
     */
    rapidjson::Value serializeMatchingRulesToJson(const MatchingRules& rules, rapidjson::Document::AllocatorType& allocator) const;
};

#endif // __GAME_RULES_CONFIG_H__
