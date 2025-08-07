#ifndef __FONT_CONFIG_H__
#define __FONT_CONFIG_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"
#include <string>

USING_NS_CC;

/**
 * 字体配置类
 * 负责管理游戏中所有字体的样式、大小等配置
 * 替代硬编码的字体设置，提供可配置的字体系统
 */
class FontConfig {
public:
    /**
     * 字体配置结构
     */
    struct FontInfo {
        std::string family;     // 字体族
        float size;             // 字体大小
        std::string text;       // 默认文本（可选）
        
        FontInfo() : family("Arial"), size(12.0f), text("") {}
        FontInfo(const std::string& fontFamily, float fontSize, const std::string& defaultText = "")
            : family(fontFamily), size(fontSize), text(defaultText) {}
    };
    
    /**
     * 构造函数
     */
    FontConfig();
    
    /**
     * 析构函数
     */
    virtual ~FontConfig();
    
    // 卡牌字体配置
    FontInfo getBigNumberFont() const { return _bigNumberFont; }
    void setBigNumberFont(const FontInfo& font) { _bigNumberFont = font; }
    
    FontInfo getSmallNumberFont() const { return _smallNumberFont; }
    void setSmallNumberFont(const FontInfo& font) { _smallNumberFont = font; }
    
    FontInfo getSuitFont() const { return _suitFont; }
    void setSuitFont(const FontInfo& font) { _suitFont = font; }
    
    FontInfo getCardBackFont() const { return _cardBackFont; }
    void setCardBackFont(const FontInfo& font) { _cardBackFont = font; }
    
    // UI字体配置
    FontInfo getTitleFont() const { return _titleFont; }
    void setTitleFont(const FontInfo& font) { _titleFont = font; }
    
    FontInfo getButtonFont() const { return _buttonFont; }
    void setButtonFont(const FontInfo& font) { _buttonFont = font; }
    
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
    // 卡牌字体配置
    FontInfo _bigNumberFont;        // 大数字字体
    FontInfo _smallNumberFont;      // 小数字字体
    FontInfo _suitFont;             // 花色字体
    FontInfo _cardBackFont;         // 卡牌背面字体
    
    // UI字体配置
    FontInfo _titleFont;            // 标题字体
    FontInfo _buttonFont;           // 按钮字体
    
    /**
     * 解析字体信息从JSON
     */
    FontInfo parseFontInfoFromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化字体信息到JSON
     */
    rapidjson::Value serializeFontInfoToJson(const FontInfo& fontInfo, rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 验证字体信息有效性
     */
    bool isValidFontInfo(const FontInfo& fontInfo) const;
};

#endif // __FONT_CONFIG_H__
