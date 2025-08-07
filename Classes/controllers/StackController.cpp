#include "StackController.h"

StackController::StackController()
    : _gameModel(nullptr)
    , _undoManager(nullptr)
    , _configManager(nullptr)
    , _currentCardView(nullptr)
    , _isInitialized(false)
    , _isProcessingOperation(false) {
}

StackController::~StackController() {
}

bool StackController::init(std::shared_ptr<GameModel> gameModel, UndoManager* undoManager) {
    if (!gameModel || !undoManager) {
        CCLOG("StackController::init - Invalid parameters");
        return false;
    }

    _gameModel = gameModel;
    _undoManager = undoManager;

    // 获取配置管理器
    _configManager = ConfigManager::getInstance();
    if (!_configManager) {
        CCLOG("StackController::init - Failed to get ConfigManager");
        return false;
    }

    _isInitialized = true;

    CCLOG("StackController::init - Initialized successfully");
    return true;
}

bool StackController::initView(const std::vector<CardView*>& stackCardViews, CardView* currentCardView) {
    if (!_isInitialized) {
        CCLOG("StackController::initView - Controller not initialized");
        return false;
    }
    
    _stackCardViews = stackCardViews;
    _currentCardView = currentCardView;
    _cardViewMap.clear();
    
    // 建立卡牌ID到视图的映射，并设置点击回调
    for (auto cardView : _stackCardViews) {
        if (cardView && cardView->getCardModel()) {
            int cardId = cardView->getCardModel()->getCardId();
            _cardViewMap[cardId] = cardView;
            
            // 设置点击回调
            cardView->setCardClickCallback([this](CardView* view, std::shared_ptr<CardModel> model) {
                onStackCardClicked(view, model);
            });
        }
    }
    
    // 更新交互性
    updateStackInteractivity();
    
    CCLOG("StackController::initView - Initialized %zu stack card views", 
          _stackCardViews.size());
    
    return true;
}

bool StackController::handleTopCardClick(const StackOperationCallback& callback) {
    if (!_isInitialized || _isProcessingOperation) {
        CCLOG("StackController::handleTopCardClick - Controller not ready");
        if (callback) callback(false, nullptr);
        return false;
    }
    
    auto topCard = getTopCard();
    if (!topCard) {
        CCLOG("StackController::handleTopCardClick - No top card available");
        if (callback) callback(false, nullptr);
        return false;
    }
    
    CCLOG("StackController::handleTopCardClick - Processing top card click: %s", 
          topCard->toString().c_str());
    
    // 执行替换操作
    _isProcessingOperation = true;
    return replaceCurrentWithTopCard([this, callback, topCard](bool success) {
        _isProcessingOperation = false;
        if (callback) callback(success, topCard);
    });
}

bool StackController::replaceCurrentWithTopCard(const AnimationCallback& callback) {
    if (!_isInitialized) {
        CCLOG("StackController::replaceCurrentWithTopCard - Controller not initialized");
        if (callback) callback(false);
        return false;
    }
    
    auto topCard = getTopCard();
    auto topCardView = getTopCardView();
    
    if (!topCard || !topCardView) {
        CCLOG("StackController::replaceCurrentWithTopCard - No top card available");
        if (callback) callback(false);
        return false;
    }
    
    // 按照README需求1：手牌区翻牌替换
    // 点击手牌区♥A，♥A会平移（简单MoveTo）到手牌区的顶部牌（♣4）并替换它作为新的顶部牌
    
    // 1. 记录撤销操作
    auto currentCard = _gameModel->getCurrentCard();
    if (!recordUndoOperation(topCard, currentCard)) {
        CCLOG("StackController::replaceCurrentWithTopCard - Failed to record undo");
        if (callback) callback(false);
        return false;
    }
    
    // 2. 更新model数据
    _gameModel->setCurrentCard(topCard);
    
    // 3. 执行动画：手牌移动到底牌位置
    auto uiLayoutConfig = _configManager->getUILayoutConfig();
    playMoveAnimation(topCardView, uiLayoutConfig->getCurrentCardPosition(), [this, callback](bool success) {
        if (success) {
            // 动画完成后，翻开下一张手牌
            revealNextCard();
            updateStackInteractivity();
            updateCurrentCardDisplay();
        }
        if (callback) callback(success);
    });
    
    CCLOG("StackController::replaceCurrentWithTopCard - Started replacement with top card: %s", 
          topCard->toString().c_str());
    
    return true;
}

bool StackController::revealNextCard() {
    if (!_gameModel) {
        return false;
    }
    
    const auto& stackCards = _gameModel->getStackCards();
    if (stackCards.empty()) {
        return false;
    }
    
    // 找到下一张需要翻开的卡牌
    for (size_t i = 0; i < stackCards.size(); i++) {
        auto card = stackCards[i];
        if (!card->isFlipped()) {
            card->setFlipped(true);
            
            // 更新对应的视图
            auto cardView = _cardViewMap[card->getCardId()];
            if (cardView) {
                cardView->setFlipped(true, true); // 带动画翻牌
            }
            
            CCLOG("StackController::revealNextCard - Revealed card: %s", card->toString().c_str());
            return true;
        }
    }
    
    return false;
}

bool StackController::hasAvailableCards() const {
    if (!_gameModel) {
        return false;
    }
    
    const auto& stackCards = _gameModel->getStackCards();
    return !stackCards.empty();
}

std::shared_ptr<CardModel> StackController::getTopCard() const {
    if (!_gameModel) {
        return nullptr;
    }
    
    const auto& stackCards = _gameModel->getStackCards();
    if (stackCards.empty()) {
        return nullptr;
    }
    
    // 返回最后一张卡牌（栈顶）
    return stackCards.back();
}

CardView* StackController::getTopCardView() const {
    auto topCard = getTopCard();
    if (!topCard) {
        return nullptr;
    }
    
    auto it = _cardViewMap.find(topCard->getCardId());
    return (it != _cardViewMap.end()) ? it->second : nullptr;
}

void StackController::updateStackDisplay() {
    // 更新所有手牌视图的显示状态
    for (auto cardView : _stackCardViews) {
        if (cardView && cardView->getCardModel()) {
            cardView->updateDisplay();
        }
    }
}

void StackController::updateCurrentCardDisplay() {
    if (_currentCardView && _gameModel) {
        auto currentCard = _gameModel->getCurrentCard();
        if (currentCard) {
            _currentCardView->setCardModel(currentCard);
            _currentCardView->updateDisplay();
        }
    }
}

bool StackController::recordUndoOperation(std::shared_ptr<CardModel> sourceCard, 
                                         std::shared_ptr<CardModel> targetCard) {
    if (!_undoManager || !sourceCard || !targetCard) {
        return false;
    }
    
    // 创建撤销记录
    auto undoModel = UndoModel::createStackToCurrentAction(
        sourceCard, targetCard,
        sourceCard->getPosition(), targetCard->getPosition()
    );
    
    return _undoManager->recordUndo(undoModel);
}

void StackController::playMoveAnimation(CardView* sourceView, const Vec2& targetPosition,
                                       const AnimationCallback& callback) {
    if (!sourceView) {
        if (callback) callback(false);
        return;
    }

    // 使用配置中的动画时长
    auto animationConfig = _configManager->getAnimationConfig();
    sourceView->playMoveAnimation(targetPosition, animationConfig->getMoveAnimationDuration(), [callback]() {
        if (callback) callback(true);
    });
}

void StackController::onStackCardClicked(CardView* cardView, std::shared_ptr<CardModel> cardModel) {
    if (!cardView || !cardModel) {
        return;
    }
    
    CCLOG("StackController::onStackCardClicked - Stack card clicked: %s", cardModel->toString().c_str());
    
    // 只有顶部卡牌可以点击
    auto topCard = getTopCard();
    if (!topCard || topCard->getCardId() != cardModel->getCardId()) {
        CCLOG("StackController::onStackCardClicked - Only top card can be clicked");
        return;
    }
    
    // 处理顶部卡牌点击
    handleTopCardClick([this](bool success, std::shared_ptr<CardModel> card) {
        if (success) {
            CCLOG("StackController::onStackCardClicked - Stack operation successful");
            // 通知外部回调
            if (_stackOperationCallback) {
                _stackOperationCallback(true, card);
            }
        } else {
            CCLOG("StackController::onStackCardClicked - Stack operation failed");
            if (_stackOperationCallback) {
                _stackOperationCallback(false, card);
            }
        }
    });
}

void StackController::updateStackInteractivity() {
    // 只有顶部卡牌可以交互
    auto topCard = getTopCard();
    
    for (auto cardView : _stackCardViews) {
        if (cardView && cardView->getCardModel()) {
            bool isTopCard = topCard && (cardView->getCardModel()->getCardId() == topCard->getCardId());
            cardView->setEnabled(isTopCard);
        }
    }
}
