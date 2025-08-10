#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include "cocos2d.h"
#include "../configs/models/UILayoutConfig.h"
#include "../configs/models/AnimationConfig.h"
#include "../configs/models/FontConfig.h"
#include "../configs/models/GameRulesConfig.h"
#include "../configs/models/CardLayoutConfig.h"
#include "../configs/models/DisplayConfig.h"
#include "../configs/loaders/LevelConfigLoader.h"
#include <memory>

USING_NS_CC;

/**
 * 配置管理器
 * 负责统一管理所有配置的加载、缓存和访问
 * 提供单一入口访问所有配置信息
 */
class ConfigManager {
public:
    /**
     * 获取单例实例
     * @return 配置管理器实例
     */
    static ConfigManager* getInstance();
    
    /**
     * 销毁单例实例
     */
    static void destroyInstance();
    
    /**
     * 初始化配置管理器
     * @return 是否初始化成功
     */
    bool init();
    
    /**
     * 加载所有配置
     * @return 是否加载成功
     */
    bool loadAllConfigs();
    
    /**
     * 重新加载所有配置
     * @return 是否重新加载成功
     */
    bool reloadAllConfigs();
    
    // 配置访问接口
    std::shared_ptr<UILayoutConfig> getUILayoutConfig() const { return _uiLayoutConfig; }
    std::shared_ptr<AnimationConfig> getAnimationConfig() const { return _animationConfig; }
    std::shared_ptr<FontConfig> getFontConfig() const { return _fontConfig; }
    std::shared_ptr<GameRulesConfig> getGameRulesConfig() const { return _gameRulesConfig; }
    std::shared_ptr<CardLayoutConfig> getCardLayoutConfig() const { return _cardLayoutConfig; }
    std::shared_ptr<DisplayConfig> getDisplayConfig() const { return _displayConfig; }
    
    /**
     * 获取关卡配置加载器
     * @return 关卡配置加载器
     */
    LevelConfigLoader* getLevelConfigLoader() { return &_levelConfigLoader; }
    
    /**
     * 获取配置摘要信息
     * @return 摘要字符串
     */
    std::string getConfigSummary() const;
    
    /**
     * 验证所有配置有效性
     * @return 是否所有配置都有效
     */
    bool validateAllConfigs() const;

protected:
    /**
     * 构造函数
     */
    ConfigManager();
    
    /**
     * 析构函数
     */
    virtual ~ConfigManager();
    
    /**
     * 加载单个配置文件
     * @param filePath 配置文件路径
     * @param config 配置对象
     * @return 是否加载成功
     */
    template<typename T>
    bool loadConfigFromFile(const std::string& filePath, std::shared_ptr<T> config);
    
    /**
     * 从JSON字符串加载配置
     * @param jsonString JSON字符串
     * @param config 配置对象
     * @return 是否加载成功
     */
    template<typename T>
    bool loadConfigFromJsonString(const std::string& jsonString, std::shared_ptr<T> config);

private:
    static ConfigManager* s_instance;               // 单例实例
    
    // 配置对象
    std::shared_ptr<UILayoutConfig> _uiLayoutConfig;        // UI布局配置
    std::shared_ptr<AnimationConfig> _animationConfig;      // 动画配置
    std::shared_ptr<FontConfig> _fontConfig;                // 字体配置
    std::shared_ptr<GameRulesConfig> _gameRulesConfig;      // 游戏规则配置
    std::shared_ptr<CardLayoutConfig> _cardLayoutConfig;    // 卡牌布局配置
    std::shared_ptr<DisplayConfig> _displayConfig;          // 显示配置
    
    // 关卡配置加载器
    LevelConfigLoader _levelConfigLoader;
    
    // 状态标志
    bool _isInitialized;                            // 是否已初始化
    bool _isLoaded;                                 // 是否已加载
    
    // 配置文件路径常量
    static const std::string kUILayoutConfigPath;
    static const std::string kAnimationConfigPath;
    static const std::string kFontConfigPath;
    static const std::string kGameRulesConfigPath;
    static const std::string kCardLayoutConfigPath;
    static const std::string kDisplayConfigPath;
};

#endif // __CONFIG_MANAGER_H__
