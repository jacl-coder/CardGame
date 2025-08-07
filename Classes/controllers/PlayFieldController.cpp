#include "PlayFieldController.h"

PlayFieldController::PlayFieldController()
    : _gameModel(nullptr)
    , _undoManager(nullptr)
    , _configManager(nullptr)
    , _isInitialized(false)
    , _isProcessingClick(false) {
}

PlayFieldController::~PlayFieldController() {
}

bool PlayFieldController::init(std::shared_ptr<GameModel> gameModel, UndoManager* undoManager) {
    if (!gameModel || !undoManager) {
        CCLOG("PlayFieldController::init - Invalid parameters");
        return false;
    }

    _gameModel = gameModel;
    _undoManager = undoManager;

    // 获取配置管理器
    _configManager = ConfigManager::getInstance();
    if (!_configManager) {
        CCLOG("PlayFieldController::init - Failed to get ConfigManager");
        return false;
    }

    _isInitialized = true;

    CCLOG("PlayFieldController::init - Initialized successfully");
    return true;
}

bool PlayFieldController::initView(const std::vector<CardView*>& playfieldCardViews) {
    if (!_isInitialized) {
        CCLOG("PlayFieldController::initView - Controller not initialized");
        return false;
    }
    
    _playfieldCardViews = playfieldCardViews;
    _cardViewMap.clear();
    
    // 建立卡牌ID到视图的映射，并设置点击回调
    for (auto cardView : _playfieldCardViews) {
        if (cardView && cardView->getCardModel()) {
            int cardId = cardView->getCardModel()->getCardId();
            _cardViewMap[cardId] = cardView;
            
            // 设置点击回调
            cardView->setCardClickCallback([this](CardView* view, std::shared_ptr<CardModel> model) {
                onCardClicked(view, model);
            });
        }
    }
    
    CCLOG("PlayFieldController::initView - Initialized %zu playfield card views", 
          _playfieldCardViews.size());
    
    return true;
}

bool PlayFieldController::handleCardClick(int cardId, const CardClickCallback& callback) {
    if (!_isInitialized || _isProcessingClick) {
        CCLOG("PlayFieldController::handleCardClick - Controller not ready");
        if (callback) callback(false, nullptr);
        return false;
    }
    
    // 查找卡牌
    auto cardView = getCardView(cardId);
    if (!cardView || !cardView->getCardModel()) {
        CCLOG("PlayFieldController::handleCardClick - Card not found: %d", cardId);
        if (callback) callback(false, nullptr);
        return false;
    }
    
    auto cardModel = cardView->getCardModel();
    
    CCLOG("PlayFieldController::handleCardClick - Processing click on card: %s", 
          cardModel->toString().c_str());
    
    // 检查卡片是否满足移动条件
    if (!checkMoveConditions(cardModel)) {
        CCLOG("PlayFieldController::handleCardClick - Move conditions not met");
        if (callback) callback(false, cardModel);
        return false;
    }
    
    // 执行替换操作
    _isProcessingClick = true;
    return replaceTrayWithPlayFieldCard(cardId, [this, callback, cardModel](bool success) {
        _isProcessingClick = false;
        if (callback) callback(success, cardModel);
    });
}

bool PlayFieldController::replaceTrayWithPlayFieldCard(int cardId, const AnimationCallback& callback) {
    if (!_isInitialized) {
        CCLOG("PlayFieldController::replaceTrayWithPlayFieldCard - Controller not initialized");
        if (callback) callback(false);
        return false;
    }
    
    auto cardView = getCardView(cardId);
    auto cardModel = cardView ? cardView->getCardModel() : nullptr;
    
    if (!cardModel) {
        CCLOG("PlayFieldController::replaceTrayWithPlayFieldCard - Invalid card: %d", cardId);
        if (callback) callback(false);
        return false;
    }
    
    // 按照README要求执行：
    // - 记录撤销操作
    // - 更新model数据  
    // - 调用相应的view执行动画
    
    // 1. 记录撤销操作
    auto currentCard = _gameModel->getCurrentCard();
    if (!recordUndoOperation(cardModel, currentCard)) {
        CCLOG("PlayFieldController::replaceTrayWithPlayFieldCard - Failed to record undo");
        if (callback) callback(false);
        return false;
    }
    
    // 2. 更新model数据
    _gameModel->setCurrentCard(cardModel);
    
    // 3. 调用相应的view执行动画
    // 使用配置中的底牌位置
    auto uiLayoutConfig = _configManager->getUILayoutConfig();
    Vec2 targetPosition = uiLayoutConfig->getCurrentCardPosition();
    playMoveAnimation(cardView, targetPosition, callback);
    
    CCLOG("PlayFieldController::replaceTrayWithPlayFieldCard - Started replacement for card: %s", 
          cardModel->toString().c_str());
    
    return true;
}

bool PlayFieldController::canMatchWithCurrentCard(std::shared_ptr<CardModel> cardModel) const {
    if (!_gameModel || !cardModel) {
        return false;
    }
    
    auto currentCard = _gameModel->getCurrentCard();
    if (!currentCard) {
        return false;
    }
    
    // 检查点数是否相差1（无花色限制）
    int currentFace = static_cast<int>(currentCard->getFace());
    int cardFace = static_cast<int>(cardModel->getFace());
    
    int diff = abs(currentFace - cardFace);
    return (diff == 1) || (diff == 12); // 考虑A和K的循环匹配
}

std::vector<std::shared_ptr<CardModel>> PlayFieldController::getMatchableCards() const {
    std::vector<std::shared_ptr<CardModel>> matchableCards;
    
    if (!_gameModel) {
        return matchableCards;
    }
    
    const auto& playfieldCards = _gameModel->getPlayfieldCards();
    for (const auto& card : playfieldCards) {
        if (canMatchWithCurrentCard(card)) {
            matchableCards.push_back(card);
        }
    }
    
    return matchableCards;
}

void PlayFieldController::highlightMatchableCards(bool highlight) {
    auto matchableCards = getMatchableCards();
    
    for (const auto& cardModel : matchableCards) {
        auto cardView = getCardView(cardModel->getCardId());
        if (cardView) {
            cardView->setHighlighted(highlight);
        }
    }
    
    CCLOG("PlayFieldController::highlightMatchableCards - %s %zu cards", 
          highlight ? "Highlighted" : "Unhighlighted", matchableCards.size());
}

CardView* PlayFieldController::getCardView(int cardId) const {
    auto it = _cardViewMap.find(cardId);
    return (it != _cardViewMap.end()) ? it->second : nullptr;
}

void PlayFieldController::updateDisplay() {
    // 更新可匹配卡牌的高亮状态
    highlightMatchableCards(false); // 先清除所有高亮
    // 可以根据需要重新高亮
}

bool PlayFieldController::checkMoveConditions(std::shared_ptr<CardModel> cardModel) const {
    if (!cardModel) {
        return false;
    }
    
    // 检查是否可以与当前底牌匹配
    return canMatchWithCurrentCard(cardModel);
}

bool PlayFieldController::recordUndoOperation(std::shared_ptr<CardModel> sourceCard, 
                                             std::shared_ptr<CardModel> targetCard) {
    if (!_undoManager || !sourceCard || !targetCard) {
        return false;
    }
    
    // 创建撤销记录
    auto undoModel = UndoModel::createPlayfieldToCurrentAction(
        sourceCard, targetCard,
        sourceCard->getPosition(), targetCard->getPosition()
    );
    
    return _undoManager->recordUndo(undoModel);
}

void PlayFieldController::playMoveAnimation(CardView* cardView, const Vec2& targetPosition,
                                           const AnimationCallback& callback) {
    if (!cardView) {
        if (callback) callback(false);
        return;
    }

    // 使用配置中的动画时长
    auto animationConfig = _configManager->getAnimationConfig();
    cardView->playMoveAnimation(targetPosition, animationConfig->getMoveAnimationDuration(), [callback]() {
        if (callback) callback(true);
    });
}

void PlayFieldController::onCardClicked(CardView* cardView, std::shared_ptr<CardModel> cardModel) {
    if (!cardView || !cardModel) {
        return;
    }
    
    CCLOG("PlayFieldController::onCardClicked - Card clicked: %s", cardModel->toString().c_str());
    
    // 处理点击事件
    handleCardClick(cardModel->getCardId(), [this](bool success, std::shared_ptr<CardModel> card) {
        if (success) {
            CCLOG("PlayFieldController::onCardClicked - Card move successful");
            // 通知外部回调
            if (_cardClickCallback) {
                _cardClickCallback(true, card);
            }
        } else {
            CCLOG("PlayFieldController::onCardClicked - Card move failed");
            if (_cardClickCallback) {
                _cardClickCallback(false, card);
            }
        }
    });
}
