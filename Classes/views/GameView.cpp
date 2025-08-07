#include "GameView.h"

GameView* GameView::create() {
    GameView* gameView = new (std::nothrow) GameView();
    if (gameView && gameView->init()) {
        gameView->autorelease();
        return gameView;
    }
    CC_SAFE_DELETE(gameView);
    return nullptr;
}

bool GameView::init() {
    if (!Layer::init()) {
        return false;
    }

    // 获取配置管理器
    _configManager = ConfigManager::getInstance();
    if (!_configManager) {
        CCLOG("GameView::init - Failed to get ConfigManager");
        return false;
    }

    // 初始化成员变量
    _currentCardView = nullptr;
    _playfieldArea = nullptr;
    _stackArea = nullptr;
    _currentCardArea = nullptr;

    return true;
}

GameView::~GameView() {
    clearAllCards();
}

bool GameView::initWithLevelConfig(std::shared_ptr<LevelConfig> levelConfig, 
                                  std::shared_ptr<GameModel> gameModel) {
    if (!levelConfig || !gameModel) {
        CCLOG("GameView::initWithLevelConfig - Invalid parameters");
        return false;
    }
    
    CCLOG("GameView::initWithLevelConfig - Initializing with level: %s", 
          levelConfig->getSummary().c_str());
    
    // 清除现有内容
    clearAllCards();
    
    // 创建背景
    createBackground(levelConfig);
    
    // 创建各个区域
    createPlayfieldArea(levelConfig, gameModel);
    createStackArea(levelConfig, gameModel);
    createCurrentCardArea(gameModel);
    
    CCLOG("GameView::initWithLevelConfig - Layout complete. Playfield: %zu cards, Stack: %zu cards", 
          _playfieldCardViews.size(), _stackCardViews.size());
    
    return true;
}

CardView* GameView::getCardView(int cardId) const {
    auto it = _cardViewMap.find(cardId);
    return (it != _cardViewMap.end()) ? it->second : nullptr;
}

void GameView::playCardMoveAnimation(CardView* cardView, const Vec2& targetPosition, 
                                    float duration, const std::function<void()>& callback) {
    if (!cardView) return;
    
    CCLOG("GameView::playCardMoveAnimation - Moving card from (%.0f,%.0f) to (%.0f,%.0f)", 
          cardView->getPosition().x, cardView->getPosition().y, 
          targetPosition.x, targetPosition.y);
    
    cardView->playMoveAnimation(targetPosition, duration, callback);
}

void GameView::updateDisplay(std::shared_ptr<GameModel> gameModel) {
    if (!gameModel) return;
    
    // 更新当前底牌
    auto currentCard = gameModel->getCurrentCard();
    if (currentCard && _currentCardView) {
        _currentCardView->setCardModel(currentCard);
        _currentCardView->updateDisplay();
    }
    
    // 这里可以添加更多的显示更新逻辑
}

void GameView::clearAllCards() {
    // 清除卡牌视图
    _playfieldCardViews.clear();
    _stackCardViews.clear();
    _currentCardView = nullptr;
    _cardViewMap.clear();
    
    // 移除所有子节点
    if (_playfieldArea) {
        _playfieldArea->removeFromParent();
        _playfieldArea = nullptr;
    }
    if (_stackArea) {
        _stackArea->removeFromParent();
        _stackArea = nullptr;
    }
    if (_currentCardArea) {
        _currentCardArea->removeFromParent();
        _currentCardArea = nullptr;
    }
}

void GameView::createPlayfieldArea(std::shared_ptr<LevelConfig> levelConfig, 
                                  std::shared_ptr<GameModel> gameModel) {
    CCLOG("GameView::createPlayfieldArea - Creating playfield with %zu cards", 
          levelConfig->getPlayfieldCards().size());
    
    // 创建桌面牌区域节点
    _playfieldArea = Node::create();
    _playfieldArea->setContentSize(levelConfig->getPlayfieldSize());
    _playfieldArea->setPosition(0, 300); // 放在屏幕上方
    addChild(_playfieldArea);
    
    // 根据配置创建桌面牌
    const auto& playfieldCards = gameModel->getPlayfieldCards();
    for (const auto& cardModel : playfieldCards) {
        auto cardView = CardView::create(cardModel);
        if (cardView) {
            // 设置卡牌位置（相对于桌面区域）
            cardView->setPosition(cardModel->getPosition());
            
            // 设置点击回调
            cardView->setCardClickCallback([this](CardView* view, std::shared_ptr<CardModel> model) {
                onCardClicked(view, model);
            });
            
            _playfieldArea->addChild(cardView);
            _playfieldCardViews.push_back(cardView);
            _cardViewMap[cardModel->getCardId()] = cardView;
            
            CCLOG("  Created playfield card: %s at (%.0f,%.0f)", 
                  cardModel->toString().c_str(), 
                  cardModel->getPosition().x, cardModel->getPosition().y);
        }
    }
}

void GameView::createStackArea(std::shared_ptr<LevelConfig> levelConfig,
                              std::shared_ptr<GameModel> gameModel) {
    CCLOG("GameView::createStackArea - Creating stack with %zu cards",
          levelConfig->getStackCards().size());

    // 创建手牌堆区域节点
    _stackArea = Node::create();
    _stackArea->setContentSize(levelConfig->getStackSize());

    // 使用配置中的手牌堆位置
    auto uiLayoutConfig = _configManager->getUILayoutConfig();
    _stackArea->setPosition(uiLayoutConfig->getStackPosition());
    addChild(_stackArea);

    // 根据配置创建手牌堆（从底部到顶部）
    const auto& stackCards = gameModel->getStackCards();
    for (size_t i = 0; i < stackCards.size(); i++) {
        const auto& cardModel = stackCards[i];
        auto cardView = CardView::create(cardModel);
        if (cardView) {
            // 手牌堆中的卡牌层叠放置，只有顶部卡牌可见
            // 使用配置中的卡牌间距
            Vec2 cardPosition = Vec2(0, i * uiLayoutConfig->getStackCardOffset());
            cardView->setPosition(cardPosition);

            // 只有顶部卡牌可以点击
            bool isTopCard = (i == stackCards.size() - 1);
            cardView->setEnabled(isTopCard);

            // 非顶部卡牌翻到背面
            if (!isTopCard) {
                cardView->setFlipped(false, false);
            }

            // 设置点击回调
            cardView->setCardClickCallback([this](CardView* view, std::shared_ptr<CardModel> model) {
                onCardClicked(view, model);
            });

            _stackArea->addChild(cardView);
            _stackCardViews.push_back(cardView);
            _cardViewMap[cardModel->getCardId()] = cardView;

            CCLOG("  Created stack card %zu: %s at (%.0f,%.0f) %s",
                  i, cardModel->toString().c_str(),
                  cardPosition.x, cardPosition.y,
                  isTopCard ? "(top)" : "(hidden)");
        }
    }
}

void GameView::createCurrentCardArea(std::shared_ptr<GameModel> gameModel) {
    CCLOG("GameView::createCurrentCardArea - Creating current card area");

    // 创建底牌区域节点
    _currentCardArea = Node::create();

    // 使用配置中的底牌位置
    auto uiLayoutConfig = _configManager->getUILayoutConfig();
    _currentCardArea->setPosition(uiLayoutConfig->getCurrentCardPosition());
    addChild(_currentCardArea);

    // 创建当前底牌
    auto currentCard = gameModel->getCurrentCard();
    if (currentCard) {
        _currentCardView = CardView::create(currentCard);
        if (_currentCardView) {
            _currentCardView->setPosition(Vec2::ZERO);

            // 底牌通常不可点击（除非有特殊规则）
            _currentCardView->setEnabled(false);

            _currentCardArea->addChild(_currentCardView);
            _cardViewMap[currentCard->getCardId()] = _currentCardView;

            CCLOG("  Created current card: %s", currentCard->toString().c_str());
        }
    }
}

void GameView::createBackground(std::shared_ptr<LevelConfig> levelConfig) {
    // 创建简单的背景
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto uiLayoutConfig = _configManager->getUILayoutConfig();

    // 桌面区域背景
    auto playfieldBg = DrawNode::create();
    playfieldBg->drawSolidRect(Vec2::ZERO,
                              Vec2(levelConfig->getPlayfieldSize().width,
                                   levelConfig->getPlayfieldSize().height),
                              uiLayoutConfig->getPlayfieldBackgroundColor().toColor4F());
    playfieldBg->setPosition(uiLayoutConfig->getPlayfieldAreaOffset());
    addChild(playfieldBg, -1);

    // 手牌区域背景
    auto stackBg = DrawNode::create();
    stackBg->drawSolidRect(Vec2::ZERO,
                          Vec2(levelConfig->getStackSize().width * uiLayoutConfig->getStackBackgroundWidthRatio(),
                               uiLayoutConfig->getStackBackgroundHeight()),
                          uiLayoutConfig->getStackBackgroundColor().toColor4F());
    stackBg->setPosition(uiLayoutConfig->getStackPosition());
    addChild(stackBg, -1);

    // 添加标题
    auto titleLabel = Label::createWithSystemFont(
        levelConfig->getLevelName().empty() ? "Card Game" : levelConfig->getLevelName(),
        "Arial", 24);
    titleLabel->setPosition(visibleSize.width * 0.5f, visibleSize.height - 50);
    titleLabel->setColor(Color3B::WHITE);
    addChild(titleLabel);

    CCLOG("GameView::createBackground - Background created for level: %s",
          levelConfig->getLevelName().c_str());
}

void GameView::onCardClicked(CardView* cardView, std::shared_ptr<CardModel> cardModel) {
    CCLOG("GameView::onCardClicked - Card clicked: %s", cardModel->toString().c_str());

    // 转发给外部回调
    if (_cardClickCallback) {
        _cardClickCallback(cardView, cardModel);
    }
}
