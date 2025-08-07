#ifndef __LEVEL_CONFIG_LOADER_H__
#define __LEVEL_CONFIG_LOADER_H__

#include "cocos2d.h"
#include "../models/LevelConfig.h"
#include <memory>
#include <string>
#include <map>

USING_NS_CC;

/**
 * 关卡配置加载器
 * 负责从文件系统加载关卡配置数据
 * 支持JSON格式的配置文件加载和缓存管理
 */
class LevelConfigLoader {
public:
    /**
     * 构造函数
     */
    LevelConfigLoader();
    
    /**
     * 析构函数
     */
    virtual ~LevelConfigLoader();
    
    /**
     * 加载指定关卡的配置
     * @param levelId 关卡ID
     * @return 关卡配置对象，加载失败返回nullptr
     */
    std::shared_ptr<LevelConfig> loadLevelConfig(int levelId);
    
    /**
     * 从文件路径加载关卡配置
     * @param filePath 配置文件路径
     * @return 关卡配置对象，加载失败返回nullptr
     */
    std::shared_ptr<LevelConfig> loadLevelConfigFromFile(const std::string& filePath);
    
    /**
     * 从JSON字符串加载关卡配置
     * @param jsonString JSON字符串
     * @param levelId 关卡ID（可选，用于设置配置的ID）
     * @return 关卡配置对象，加载失败返回nullptr
     */
    std::shared_ptr<LevelConfig> loadLevelConfigFromString(const std::string& jsonString, int levelId = 0);
    
    /**
     * 预加载所有关卡配置
     * @param configDirectory 配置文件目录
     * @return 成功加载的关卡数量
     */
    int preloadAllLevelConfigs(const std::string& configDirectory = "configs/data/levels/");
    
    /**
     * 获取已加载的关卡配置
     * @param levelId 关卡ID
     * @return 关卡配置对象，未找到返回nullptr
     */
    std::shared_ptr<LevelConfig> getCachedLevelConfig(int levelId) const;
    
    /**
     * 清除所有缓存的配置
     */
    void clearCache();
    
    /**
     * 获取已加载的关卡数量
     * @return 关卡数量
     */
    int getLoadedLevelCount() const;
    
    /**
     * 获取所有已加载的关卡ID列表
     * @return 关卡ID列表
     */
    std::vector<int> getLoadedLevelIds() const;
    
    /**
     * 验证关卡配置文件格式
     * @param filePath 配置文件路径
     * @return 是否为有效的配置文件
     */
    bool validateConfigFile(const std::string& filePath) const;
    

    /**
     * 保存关卡配置到文件
     * @param levelConfig 关卡配置
     * @param filePath 保存路径
     * @return 是否保存成功
     */
    bool saveLevelConfig(std::shared_ptr<LevelConfig> levelConfig, const std::string& filePath) const;

private:
    std::map<int, std::shared_ptr<LevelConfig>> _cachedConfigs;  // 缓存的配置
    
    /**
     * 解析JSON文档
     * @param jsonString JSON字符串
     * @param document 输出的JSON文档
     * @return 是否解析成功
     */
    bool parseJsonDocument(const std::string& jsonString, rapidjson::Document& document) const;
    
    /**
     * 从文件读取内容
     * @param filePath 文件路径
     * @return 文件内容，读取失败返回空字符串
     */
    std::string readFileContent(const std::string& filePath) const;
    
    /**
     * 获取关卡配置文件路径
     * @param levelId 关卡ID
     * @return 配置文件路径
     */
    std::string getLevelConfigFilePath(int levelId) const;
    
    /**
     * 验证JSON文档格式
     * @param document JSON文档
     * @return 是否为有效的关卡配置格式
     */
    bool validateJsonDocument(const rapidjson::Document& document) const;
};

#endif // __LEVEL_CONFIG_LOADER_H__
