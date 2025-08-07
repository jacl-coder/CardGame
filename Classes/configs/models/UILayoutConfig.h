#ifndef __UI_LAYOUT_CONFIG_H__
#define __UI_LAYOUT_CONFIG_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"

USING_NS_CC;

/**
 * UI布局配置类
 * 负责管理游戏中所有UI元素的位置、间距、颜色等布局相关配置
 * 替代硬编码的布局常量，提供可配置的UI布局系统
 */
class UILayoutConfig {
public:
    /**
     * 颜色配置结构
     */
    struct ColorConfig {
        float r, g, b, a;
        
        ColorConfig() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
        ColorConfig(float red, float green, float blue, float alpha) 
            : r(red), g(green), b(blue), a(alpha) {}
        
        Color4F toColor4F() const { return Color4F(r, g, b, a); }
        Color3B toColor3B() const { return Color3B(r * 255, g * 255, b * 255); }
    };
    
    /**
     * 构造函数
     */
    UILayoutConfig();
    
    /**
     * 析构函数
     */
    virtual ~UILayoutConfig();
    
    // 位置配置
    Vec2 getStackPosition() const { return _stackPosition; }
    void setStackPosition(const Vec2& position) { _stackPosition = position; }
    
    Vec2 getCurrentCardPosition() const { return _currentCardPosition; }
    void setCurrentCardPosition(const Vec2& position) { _currentCardPosition = position; }
    
    Vec2 getPlayfieldAreaOffset() const { return _playfieldAreaOffset; }
    void setPlayfieldAreaOffset(const Vec2& offset) { _playfieldAreaOffset = offset; }
    
    // 间距配置
    float getStackCardOffset() const { return _stackCardOffset; }
    void setStackCardOffset(float offset) { _stackCardOffset = offset; }
    
    // 背景颜色配置
    ColorConfig getPlayfieldBackgroundColor() const { return _playfieldBgColor; }
    void setPlayfieldBackgroundColor(const ColorConfig& color) { _playfieldBgColor = color; }
    
    ColorConfig getStackBackgroundColor() const { return _stackBgColor; }
    void setStackBackgroundColor(const ColorConfig& color) { _stackBgColor = color; }
    
    // 背景尺寸配置
    float getStackBackgroundWidthRatio() const { return _stackBgWidthRatio; }
    void setStackBackgroundWidthRatio(float ratio) { _stackBgWidthRatio = ratio; }
    
    float getStackBackgroundHeight() const { return _stackBgHeight; }
    void setStackBackgroundHeight(float height) { _stackBgHeight = height; }
    
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
    // 位置配置
    Vec2 _stackPosition;                    // 手牌堆位置
    Vec2 _currentCardPosition;              // 底牌位置
    Vec2 _playfieldAreaOffset;              // 桌面区域偏移
    
    // 间距配置
    float _stackCardOffset;                 // 手牌堆卡牌间距
    
    // 背景颜色配置
    ColorConfig _playfieldBgColor;          // 桌面背景颜色
    ColorConfig _stackBgColor;              // 手牌堆背景颜色
    
    // 背景尺寸配置
    float _stackBgWidthRatio;               // 手牌堆背景宽度比例
    float _stackBgHeight;                   // 手牌堆背景高度
    
    /**
     * 解析Vec2从JSON
     */
    Vec2 parseVec2FromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化Vec2到JSON
     */
    rapidjson::Value serializeVec2ToJson(const Vec2& vec, rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 解析颜色从JSON
     */
    ColorConfig parseColorFromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化颜色到JSON
     */
    rapidjson::Value serializeColorToJson(const ColorConfig& color, rapidjson::Document::AllocatorType& allocator) const;
};

#endif // __UI_LAYOUT_CONFIG_H__
