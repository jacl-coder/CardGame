#include "GameModelFromLevelGenerator.h"
#include <algorithm>
#include <random>

int GameModelFromLevelGenerator::s_nextCardId = 1000; // 默认值，将从配置中读取

std::shared_ptr<GameModel> GameModelFromLevelGenerator::generateGameModel(std::shared_ptr<LevelConfig> levelConfig) {
    return generateGameModel(levelConfig, false, false);
}

std::shared_ptr<GameModel> GameModelFromLevelGenerator::generateGameModel(std::shared_ptr<LevelConfig> levelConfig,
                                                                         bool shufflePlayfield,
                                                                         bool shuffleStack) {
    if (!validateLevelConfig(levelConfig)) {
        CCLOG("GameModelFromLevelGenerator::generateGameModel - Invalid level config");
        return nullptr;
    }
    
    CCLOG("GameModelFromLevelGenerator::generateGameModel - Generating game model for: %s", 
          levelConfig->getSummary().c_str());
    
    // 创建游戏模型
    auto gameModel = std::make_shared<GameModel>();
    
    // 生成桌面牌
    if (!generatePlayfieldCards(levelConfig, gameModel, shufflePlayfield)) {
        CCLOG("GameModelFromLevelGenerator::generateGameModel - Failed to generate playfield cards");
        return nullptr;
    }
    
    // 生成手牌堆
    if (!generateStackCards(levelConfig, gameModel, shuffleStack)) {
        CCLOG("GameModelFromLevelGenerator::generateGameModel - Failed to generate stack cards");
        return nullptr;
    }
    
    // 设置初始底牌
    if (!setInitialCurrentCard(gameModel)) {
        CCLOG("GameModelFromLevelGenerator::generateGameModel - Failed to set initial current card");
        return nullptr;
    }
    
    // 设置游戏状态
    gameModel->setGameState(GameState::INITIALIZING);
    
    CCLOG("GameModelFromLevelGenerator::generateGameModel - %s", 
          getGenerationSummary(gameModel).c_str());
    
    return gameModel;
}

bool GameModelFromLevelGenerator::validateLevelConfig(std::shared_ptr<LevelConfig> levelConfig) {
    if (!levelConfig) {
        return false;
    }
    
    // 检查是否有桌面牌
    if (levelConfig->getPlayfieldCards().empty()) {
        CCLOG("GameModelFromLevelGenerator::validateLevelConfig - No playfield cards");
        return false;
    }
    
    // 检查是否有手牌堆
    if (levelConfig->getStackCards().empty()) {
        CCLOG("GameModelFromLevelGenerator::validateLevelConfig - No stack cards");
        return false;
    }
    
    // 验证每张卡牌的配置
    for (const auto& cardConfig : levelConfig->getPlayfieldCards()) {
        if (!validateCardConfigData(cardConfig)) {
            return false;
        }
    }
    
    for (const auto& cardConfig : levelConfig->getStackCards()) {
        if (!validateCardConfigData(cardConfig)) {
            return false;
        }
    }
    
    return true;
}

bool GameModelFromLevelGenerator::generatePlayfieldCards(std::shared_ptr<LevelConfig> levelConfig,
                                                        std::shared_ptr<GameModel> gameModel,
                                                        bool shuffle) {
    if (!levelConfig || !gameModel) {
        return false;
    }
    
    gameModel->clearPlayfieldCards();
    
    // 从配置创建桌面牌
    std::vector<std::shared_ptr<CardModel>> playfieldCards;
    for (const auto& configData : levelConfig->getPlayfieldCards()) {
        auto cardModel = createCardFromConfig(configData);
        if (cardModel) {
            setupCardGameProperties(cardModel, true);
            playfieldCards.push_back(cardModel);
        }
    }
    
    // 如果需要打乱
    if (shuffle) {
        shuffleCards(playfieldCards);
    }
    
    // 添加到游戏模型
    for (auto& card : playfieldCards) {
        gameModel->addPlayfieldCard(card);
    }
    
    CCLOG("GameModelFromLevelGenerator::generatePlayfieldCards - Generated %zu playfield cards", 
          playfieldCards.size());
    
    return true;
}

bool GameModelFromLevelGenerator::generateStackCards(std::shared_ptr<LevelConfig> levelConfig,
                                                    std::shared_ptr<GameModel> gameModel,
                                                    bool shuffle) {
    if (!levelConfig || !gameModel) {
        return false;
    }
    
    gameModel->clearStackCards();
    
    // 从配置创建手牌堆
    std::vector<std::shared_ptr<CardModel>> stackCards;
    for (const auto& configData : levelConfig->getStackCards()) {
        auto cardModel = createCardFromConfig(configData);
        if (cardModel) {
            setupCardGameProperties(cardModel, false);
            stackCards.push_back(cardModel);
        }
    }
    
    // 如果需要打乱
    if (shuffle) {
        shuffleCards(stackCards);
    }
    
    // 添加到游戏模型
    for (auto& card : stackCards) {
        gameModel->addStackCard(card);
    }
    
    CCLOG("GameModelFromLevelGenerator::generateStackCards - Generated %zu stack cards", 
          stackCards.size());
    
    return true;
}

bool GameModelFromLevelGenerator::setInitialCurrentCard(std::shared_ptr<GameModel> gameModel) {
    if (!gameModel) {
        return false;
    }
    
    // 使用手牌堆的第一张作为初始底牌
    const auto& stackCards = gameModel->getStackCards();
    if (stackCards.empty()) {
        CCLOG("GameModelFromLevelGenerator::setInitialCurrentCard - No stack cards available");
        return false;
    }
    
    auto firstStackCard = stackCards[0];
    auto currentCard = std::make_shared<CardModel>(firstStackCard->getFace(), 
                                                  firstStackCard->getSuit(), 
                                                  Vec2::ZERO);
    currentCard->setCardId(generateUniqueCardId());
    currentCard->setFlipped(true); // 底牌始终正面朝上
    
    gameModel->setCurrentCard(currentCard);
    
    CCLOG("GameModelFromLevelGenerator::setInitialCurrentCard - Set initial current card: %s", 
          currentCard->toString().c_str());
    
    return true;
}

void GameModelFromLevelGenerator::shuffleCards(std::vector<std::shared_ptr<CardModel>>& cards) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(cards.begin(), cards.end(), g);
}

std::shared_ptr<CardModel> GameModelFromLevelGenerator::createCardFromConfig(const CardConfigData& configData) {
    auto cardModel = std::make_shared<CardModel>(configData.cardFace, configData.cardSuit, configData.position);
    cardModel->setCardId(generateUniqueCardId());
    return cardModel;
}

std::string GameModelFromLevelGenerator::getGenerationSummary(std::shared_ptr<GameModel> gameModel) {
    if (!gameModel) {
        return "Invalid game model";
    }
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), 
             "Generated game model: %zu playfield cards, %zu stack cards, current card: %s",
             gameModel->getPlayfieldCards().size(),
             gameModel->getStackCards().size(),
             gameModel->getCurrentCard() ? gameModel->getCurrentCard()->toString().c_str() : "None");
    
    return std::string(buffer);
}

int GameModelFromLevelGenerator::generateUniqueCardId() {
    // 首次调用时从配置中初始化起始ID
    static bool initialized = false;
    if (!initialized) {
        auto configManager = getConfigManager();
        if (configManager) {
            auto gameRulesConfig = configManager->getGameRulesConfig();
            s_nextCardId = gameRulesConfig->getStartingCardId();
        }
        initialized = true;
    }

    return s_nextCardId++;
}

ConfigManager* GameModelFromLevelGenerator::getConfigManager() {
    return ConfigManager::getInstance();
}

bool GameModelFromLevelGenerator::validateCardConfigData(const CardConfigData& configData) {
    // 检查牌面类型
    if (static_cast<int>(configData.cardFace) < 0 || 
        static_cast<int>(configData.cardFace) >= CFT_NUM_CARD_FACE_TYPES) {
        return false;
    }
    
    // 检查花色类型
    if (static_cast<int>(configData.cardSuit) < 0 || 
        static_cast<int>(configData.cardSuit) >= CST_NUM_CARD_SUIT_TYPES) {
        return false;
    }
    
    return true;
}

void GameModelFromLevelGenerator::setupCardGameProperties(std::shared_ptr<CardModel> cardModel, bool isPlayfieldCard) {
    if (!cardModel) {
        return;
    }
    
    // 桌面牌默认正面朝上，手牌堆中的卡牌根据位置决定
    cardModel->setFlipped(true);
    
    // 可以在这里设置其他游戏属性
    // 例如：可点击性、动画状态等
}
