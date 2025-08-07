#ifndef __ANIMATION_CONFIG_H__
#define __ANIMATION_CONFIG_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"

USING_NS_CC;

/**
 * 动画配置类
 * 负责管理游戏中所有动画的时长、效果等配置
 * 替代硬编码的动画常量，提供可配置的动画系统
 */
class AnimationConfig {
public:
    /**
     * 构造函数
     */
    AnimationConfig();
    
    /**
     * 析构函数
     */
    virtual ~AnimationConfig();
    
    // 动画时长配置
    float getMoveAnimationDuration() const { return _moveAnimationDuration; }
    void setMoveAnimationDuration(float duration) { _moveAnimationDuration = duration; }
    
    float getFlipAnimationDuration() const { return _flipAnimationDuration; }
    void setFlipAnimationDuration(float duration) { _flipAnimationDuration = duration; }
    
    float getScaleAnimationDuration() const { return _scaleAnimationDuration; }
    void setScaleAnimationDuration(float duration) { _scaleAnimationDuration = duration; }
    
    float getHighlightAnimationDuration() const { return _highlightAnimationDuration; }
    void setHighlightAnimationDuration(float duration) { _highlightAnimationDuration = duration; }
    
    // 动画效果配置
    float getHighlightScaleFactor() const { return _highlightScaleFactor; }
    void setHighlightScaleFactor(float factor) { _highlightScaleFactor = factor; }
    
    float getClickScaleFactor() const { return _clickScaleFactor; }
    void setClickScaleFactor(float factor) { _clickScaleFactor = factor; }
    
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
    // 动画时长配置
    float _moveAnimationDuration;           // 移动动画时长
    float _flipAnimationDuration;           // 翻牌动画时长
    float _scaleAnimationDuration;          // 缩放动画时长
    float _highlightAnimationDuration;      // 高亮动画时长
    
    // 动画效果配置
    float _highlightScaleFactor;            // 高亮缩放因子
    float _clickScaleFactor;                // 点击缩放因子
};

#endif // __ANIMATION_CONFIG_H__
