#ifndef __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
#define __GAME_MODEL_FROM_LEVEL_GENERATOR_H__

#include "cocos2d.h"
#include "../models/GameModel.h"
#include "../configs/models/LevelConfig.h"
#include "../managers/ConfigManager.h"
#include <memory>

USING_NS_CC;

/**
 * 游戏模型生成服务
 * 将静态配置（LevelConfig）转换为动态运行时数据（GameModel）
 * 处理卡牌随机生成策略等业务逻辑
 * 
 * 服务层特点：
 * - 提供无状态的服务，不管理数据生命周期
 * - 不持有数据，通过参数操作数据或返回数据
 * - 可以实现为单例或提供静态方法
 * - 不依赖Controllers，提供基础服务
 */
class GameModelFromLevelGenerator {
public:
    /**
     * 从关卡配置生成游戏模型
     * @param levelConfig 关卡配置
     * @return 生成的游戏模型，失败返回nullptr
     */
    static std::shared_ptr<GameModel> generateGameModel(std::shared_ptr<LevelConfig> levelConfig);
    
    /**
     * 从关卡配置生成游戏模型（带自定义参数）
     * @param levelConfig 关卡配置
     * @param shufflePlayfield 是否打乱桌面牌
     * @param shuffleStack 是否打乱手牌堆
     * @return 生成的游戏模型，失败返回nullptr
     */
    static std::shared_ptr<GameModel> generateGameModel(std::shared_ptr<LevelConfig> levelConfig,
                                                       bool shufflePlayfield,
                                                       bool shuffleStack);
    
    /**
     * 验证关卡配置的有效性
     * @param levelConfig 关卡配置
     * @return 是否有效
     */
    static bool validateLevelConfig(std::shared_ptr<LevelConfig> levelConfig);
    
    /**
     * 生成桌面牌数据
     * @param levelConfig 关卡配置
     * @param gameModel 目标游戏模型
     * @param shuffle 是否打乱
     * @return 是否生成成功
     */
    static bool generatePlayfieldCards(std::shared_ptr<LevelConfig> levelConfig,
                                      std::shared_ptr<GameModel> gameModel,
                                      bool shuffle = false);
    
    /**
     * 生成手牌堆数据
     * @param levelConfig 关卡配置
     * @param gameModel 目标游戏模型
     * @param shuffle 是否打乱
     * @return 是否生成成功
     */
    static bool generateStackCards(std::shared_ptr<LevelConfig> levelConfig,
                                  std::shared_ptr<GameModel> gameModel,
                                  bool shuffle = false);
    
    /**
     * 设置初始底牌
     * @param gameModel 游戏模型
     * @return 是否设置成功
     */
    static bool setInitialCurrentCard(std::shared_ptr<GameModel> gameModel);
    
    /**
     * 打乱卡牌数组
     * @param cards 卡牌数组
     */
    static void shuffleCards(std::vector<std::shared_ptr<CardModel>>& cards);
    
    /**
     * 从配置数据创建卡牌模型
     * @param configData 配置数据
     * @return 卡牌模型
     */
    static std::shared_ptr<CardModel> createCardFromConfig(const CardConfigData& configData);
    
    /**
     * 获取生成统计信息
     * @param gameModel 游戏模型
     * @return 统计信息字符串
     */
    static std::string getGenerationSummary(std::shared_ptr<GameModel> gameModel);

private:
    /**
     * 私有构造函数，防止实例化
     */
    GameModelFromLevelGenerator() = delete;
    
    /**
     * 生成唯一的卡牌ID
     * @return 卡牌ID
     */
    static int generateUniqueCardId();
    
    /**
     * 验证卡牌配置数据
     * @param configData 配置数据
     * @return 是否有效
     */
    static bool validateCardConfigData(const CardConfigData& configData);
    
    /**
     * 设置卡牌的游戏属性
     * @param cardModel 卡牌模型
     * @param isPlayfieldCard 是否为桌面牌
     */
    static void setupCardGameProperties(std::shared_ptr<CardModel> cardModel, bool isPlayfieldCard);
    
    // 静态计数器
    static int s_nextCardId;

    /**
     * 获取配置管理器
     * @return 配置管理器实例
     */
    static ConfigManager* getConfigManager();
};

#endif // __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
