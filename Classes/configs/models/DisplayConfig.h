#ifndef __DISPLAY_CONFIG_H__
#define __DISPLAY_CONFIG_H__

#include "cocos2d.h"
#include "external/json/rapidjson.h"
#include "external/json/document.h"
#include <string>
#include <vector>

USING_NS_CC;

/**
 * 显示配置类
 * 负责管理显示相关的配置，包括分辨率、窗口设置等
 * 替代硬编码的显示常量，提供可配置的显示系统
 */
class DisplayConfig {
public:
    /**
     * 分辨率配置结构
     */
    struct ResolutionInfo {
        std::string name;       // 分辨率名称
        int width;              // 宽度
        int height;             // 高度
        
        ResolutionInfo() : name("default"), width(1080), height(2080) {}
        ResolutionInfo(const std::string& resName, int w, int h) 
            : name(resName), width(w), height(h) {}
        
        Size toSize() const { return Size(width, height); }
    };
    
    /**
     * 构造函数
     */
    DisplayConfig();
    
    /**
     * 析构函数
     */
    virtual ~DisplayConfig();
    
    // 设计分辨率配置
    ResolutionInfo getDesignResolution() const { return _designResolution; }
    void setDesignResolution(const ResolutionInfo& resolution) { _designResolution = resolution; }
    
    // 窗口配置
    float getWindowScale() const { return _windowScale; }
    void setWindowScale(float scale) { _windowScale = scale; }
    
    std::string getResolutionPolicy() const { return _resolutionPolicy; }
    void setResolutionPolicy(const std::string& policy) { _resolutionPolicy = policy; }
    
    std::string getWindowTitle() const { return _windowTitle; }
    void setWindowTitle(const std::string& title) { _windowTitle = title; }
    
    // 支持的分辨率配置
    std::vector<ResolutionInfo> getSupportedResolutions() const { return _supportedResolutions; }
    void setSupportedResolutions(const std::vector<ResolutionInfo>& resolutions) { _supportedResolutions = resolutions; }
    
    // 便捷方法
    Size getDesignResolutionSize() const { return _designResolution.toSize(); }
    ResolutionPolicy getResolutionPolicyType() const;
    ResolutionInfo getResolutionByName(const std::string& name) const;
    
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
    ResolutionInfo _designResolution;               // 设计分辨率
    float _windowScale;                             // 窗口缩放比例
    std::string _resolutionPolicy;                  // 分辨率策略
    std::string _windowTitle;                       // 窗口标题
    std::vector<ResolutionInfo> _supportedResolutions; // 支持的分辨率列表
    
    /**
     * 解析分辨率信息从JSON
     */
    ResolutionInfo parseResolutionInfoFromJson(const rapidjson::Value& json) const;
    
    /**
     * 序列化分辨率信息到JSON
     */
    rapidjson::Value serializeResolutionInfoToJson(const ResolutionInfo& resolution, rapidjson::Document::AllocatorType& allocator) const;
    
    /**
     * 验证分辨率信息有效性
     */
    bool isValidResolutionInfo(const ResolutionInfo& resolution) const;
    
    /**
     * 解析分辨率策略字符串
     */
    ResolutionPolicy parseResolutionPolicy(const std::string& policy) const;
};

#endif // __DISPLAY_CONFIG_H__
