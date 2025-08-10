#include "ConfigManager.h"
#include "external/json/document.h"

// 静态成员初始化
ConfigManager* ConfigManager::s_instance = nullptr;

// 配置文件路径常量（相对于Resources目录）
const std::string ConfigManager::kUILayoutConfigPath = "configs/data/ui/layout_config.json";
const std::string ConfigManager::kAnimationConfigPath = "configs/data/ui/animation_config.json";
const std::string ConfigManager::kFontConfigPath = "configs/data/ui/font_config.json";
const std::string ConfigManager::kGameRulesConfigPath = "configs/data/game/rules_config.json";
const std::string ConfigManager::kCardLayoutConfigPath = "configs/data/game/card_layout_config.json";
const std::string ConfigManager::kDisplayConfigPath = "configs/data/display/display_config.json";

ConfigManager* ConfigManager::getInstance() {
    if (!s_instance) {
        s_instance = new ConfigManager();
    }
    return s_instance;
}

void ConfigManager::destroyInstance() {
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

ConfigManager::ConfigManager()
    : _isInitialized(false)
    , _isLoaded(false) {
}

ConfigManager::~ConfigManager() {
}

bool ConfigManager::init() {
    if (_isInitialized) {
        return true;
    }

    // 创建配置对象
    _uiLayoutConfig = std::make_shared<UILayoutConfig>();
    _animationConfig = std::make_shared<AnimationConfig>();
    _fontConfig = std::make_shared<FontConfig>();
    _gameRulesConfig = std::make_shared<GameRulesConfig>();
    _cardLayoutConfig = std::make_shared<CardLayoutConfig>();
    _displayConfig = std::make_shared<DisplayConfig>();

    _isInitialized = true;
    CCLOG("ConfigManager::init - Initialized successfully");

    return true;
}

bool ConfigManager::loadAllConfigs() {
    if (!_isInitialized) {
        CCLOG("ConfigManager::loadAllConfigs - Not initialized");
        return false;
    }
    
    CCLOG("ConfigManager::loadAllConfigs - Loading all configurations...");
    
    bool allSuccess = true;
    
    // 加载UI布局配置
    if (!loadConfigFromFile(kUILayoutConfigPath, _uiLayoutConfig)) {
        CCLOG("ConfigManager::loadAllConfigs - Failed to load UI layout config, using defaults");
        _uiLayoutConfig->resetToDefault();
        allSuccess = false;
    }
    
    // 加载动画配置
    if (!loadConfigFromFile(kAnimationConfigPath, _animationConfig)) {
        CCLOG("ConfigManager::loadAllConfigs - Failed to load animation config, using defaults");
        _animationConfig->resetToDefault();
        allSuccess = false;
    }
    
    // 加载字体配置
    if (!loadConfigFromFile(kFontConfigPath, _fontConfig)) {
        CCLOG("ConfigManager::loadAllConfigs - Failed to load font config, using defaults");
        _fontConfig->resetToDefault();
        allSuccess = false;
    }
    
    // 加载游戏规则配置
    if (!loadConfigFromFile(kGameRulesConfigPath, _gameRulesConfig)) {
        CCLOG("ConfigManager::loadAllConfigs - Failed to load game rules config, using defaults");
        _gameRulesConfig->resetToDefault();
        allSuccess = false;
    }
    
    // 加载卡牌布局配置
    if (!loadConfigFromFile(kCardLayoutConfigPath, _cardLayoutConfig)) {
        CCLOG("ConfigManager::loadAllConfigs - Failed to load card layout config, using defaults");
        _cardLayoutConfig->resetToDefault();
        allSuccess = false;
    }
    
    // 加载显示配置
    if (!loadConfigFromFile(kDisplayConfigPath, _displayConfig)) {
        CCLOG("ConfigManager::loadAllConfigs - Failed to load display config, using defaults");
        _displayConfig->resetToDefault();
        allSuccess = false;
    }
    
    // 验证所有配置
    if (!validateAllConfigs()) {
        CCLOG("ConfigManager::loadAllConfigs - Some configurations are invalid");
        allSuccess = false;
    }
    
    _isLoaded = allSuccess;
    
    if (allSuccess) {
        CCLOG("ConfigManager::loadAllConfigs - All configurations loaded successfully");
        CCLOG("ConfigManager::loadAllConfigs - %s", getConfigSummary().c_str());
    } else {
        CCLOG("ConfigManager::loadAllConfigs - Some configurations failed to load, using defaults");
    }
    
    return allSuccess;
}

bool ConfigManager::reloadAllConfigs() {
    CCLOG("ConfigManager::reloadAllConfigs - Reloading all configurations...");
    _isLoaded = false;
    return loadAllConfigs();
}

std::string ConfigManager::getConfigSummary() const {
    std::string summary = "ConfigManager Summary:\n";
    
    if (_uiLayoutConfig) {
        summary += "  " + _uiLayoutConfig->getSummary() + "\n";
    }
    
    if (_animationConfig) {
        summary += "  " + _animationConfig->getSummary() + "\n";
    }
    
    if (_fontConfig) {
        summary += "  " + _fontConfig->getSummary() + "\n";
    }
    
    if (_gameRulesConfig) {
        summary += "  " + _gameRulesConfig->getSummary() + "\n";
    }
    
    if (_cardLayoutConfig) {
        summary += "  " + _cardLayoutConfig->getSummary() + "\n";
    }
    
    if (_displayConfig) {
        summary += "  " + _displayConfig->getSummary() + "\n";
    }
    
    return summary;
}

bool ConfigManager::validateAllConfigs() const {
    return _uiLayoutConfig && _uiLayoutConfig->isValid() &&
           _animationConfig && _animationConfig->isValid() &&
           _fontConfig && _fontConfig->isValid() &&
           _gameRulesConfig && _gameRulesConfig->isValid() &&
           _cardLayoutConfig && _cardLayoutConfig->isValid() &&
           _displayConfig && _displayConfig->isValid();
}

template<typename T>
bool ConfigManager::loadConfigFromFile(const std::string& filePath, std::shared_ptr<T> config) {
    if (!config) {
        CCLOG("ConfigManager::loadConfigFromFile - Invalid config object for %s", filePath.c_str());
        return false;
    }
    
    // 读取文件内容
    std::string jsonString = FileUtils::getInstance()->getStringFromFile(filePath);
    if (jsonString.empty()) {
        CCLOG("ConfigManager::loadConfigFromFile - Failed to read file: %s", filePath.c_str());
        return false;
    }
    
    return loadConfigFromJsonString(jsonString, config);
}

template<typename T>
bool ConfigManager::loadConfigFromJsonString(const std::string& jsonString, std::shared_ptr<T> config) {
    if (!config) {
        return false;
    }
    
    // 解析JSON
    rapidjson::Document document;
    document.Parse(jsonString.c_str());
    
    if (document.HasParseError()) {
        CCLOG("ConfigManager::loadConfigFromJsonString - JSON parse error: %d", document.GetParseError());
        return false;
    }
    
    // 加载配置
    return config->fromJson(document);
}

// 显式实例化模板
template bool ConfigManager::loadConfigFromFile<UILayoutConfig>(const std::string&, std::shared_ptr<UILayoutConfig>);
template bool ConfigManager::loadConfigFromFile<AnimationConfig>(const std::string&, std::shared_ptr<AnimationConfig>);
template bool ConfigManager::loadConfigFromFile<FontConfig>(const std::string&, std::shared_ptr<FontConfig>);
template bool ConfigManager::loadConfigFromFile<GameRulesConfig>(const std::string&, std::shared_ptr<GameRulesConfig>);
template bool ConfigManager::loadConfigFromFile<CardLayoutConfig>(const std::string&, std::shared_ptr<CardLayoutConfig>);
template bool ConfigManager::loadConfigFromFile<DisplayConfig>(const std::string&, std::shared_ptr<DisplayConfig>);
