#include "StackController.h"
#include <algorithm>

StackController::StackController()
    : _gameModel(nullptr)
    , _undoManager(nullptr)
    , _configManager(nullptr)
    , _currentCardView(nullptr)
    , _isInitialized(false)
    , _isProcessingOperation(false)
    , _initialDealt(false) {
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
    if (!_isInitialized) {
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
    
    // 执行替换操作（允许并发）
    return replaceCurrentWithTopCard([this, callback, topCard](bool success) {
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
    
    // 1. 记录撤销操作
    auto currentCard = _gameModel->getCurrentCard();
    if (!recordUndoOperation(topCard, currentCard)) {
        CCLOG("StackController::replaceCurrentWithTopCard - Failed to record undo");
        if (callback) callback(false);
        return false;
    }
    
    // 2. 更新model数据 - 先添加到底牌栈（但不从手牌栈移除）
    _gameModel->pushCurrentCard(topCard);
    
    // 打印栈信息
    const auto& stack = _gameModel->getCurrentCardStack();
    CCLOG("StackController - Stack size after push: %zu", stack.size());
    for (size_t i = 0; i < stack.size(); i++) {
        CCLOG("  Stack[%zu]: %s", i, stack[i]->toString().c_str());
    }
    CCLOG("StackController - Hand cards remaining (before animation): %zu", _gameModel->getStackCards().size());
    
    // 3. 执行动画：顶部手牌移动到配置的底牌位置（不依赖 _currentCardView，避免悬挂指针）
    auto uiLayoutConfig = _configManager->getUILayoutConfig();

    cocos2d::Node* srcParent = topCardView->getParent();
    // 选择 GameView 作为 overlayParent：stackArea 的父节点通常即 GameView
    cocos2d::Node* overlayParent = (srcParent && srcParent->getParent()) ? srcParent->getParent() : srcParent;

    cocos2d::Vec2 worldStart = srcParent->convertToWorldSpace(topCardView->getPosition());
    // 目标直接使用配置坐标（GameView坐标）
    cocos2d::Vec2 worldTarget = uiLayoutConfig->getCurrentCardPosition();

    cocos2d::Vec2 startInOverlay = overlayParent->convertToNodeSpace(worldStart);
    cocos2d::Vec2 targetInOverlay = overlayParent->convertToNodeSpace(worldTarget);

    // 在动画开始前，立即从『模型与容器』移除栈顶手牌，以便立刻启用下一张手牌（支持连续点击）
    int topCardId = topCard->getCardId();
    _gameModel->removeTopStackCard();

    // 从控制器容器中移除旧的视图引用，避免残留悬空指针
    auto mapItPre = _cardViewMap.find(topCardId);
    if (mapItPre != _cardViewMap.end()) {
        _cardViewMap.erase(mapItPre);
    }
    auto vecItPre = std::find(_stackCardViews.begin(), _stackCardViews.end(), topCardView);
    if (vecItPre != _stackCardViews.end()) {
        _stackCardViews.erase(vecItPre);
    }

    // 立即翻开下一张手牌并启用交互（如果存在）
    revealNextCard();
    updateStackInteractivity();

    // 提升层级并移动，确保覆盖显示（动画期间该视图独立存在，不再受手牌容器管理）
    topCardView->retain();
    topCardView->removeFromParent();
    overlayParent->addChild(topCardView, 500); // 动画层
    topCardView->setPosition(startInOverlay);
    topCardView->setEnabled(false); // 动画期间禁用交互

    playMoveAnimation(topCardView, targetInOverlay, [this, callback, topCardId, topCardView](bool success) {
        // 安全检查：确保对象仍然有效
        if (!this || !_gameModel) {
            CCLOG("StackController::replaceCurrentWithTopCard - Animation callback: Controller or model invalid!");
            if (topCardView) {
                topCardView->removeFromParent();
                topCardView->release();
            }
            if (callback) callback(false);
            return;
        }
        
        if (topCardView) {
            // 在释放视图前，先从本控制器的容器中移除，避免残留悬空指针
            auto mapIt = _cardViewMap.find(topCardId);
            if (mapIt != _cardViewMap.end()) {
                _cardViewMap.erase(mapIt);
            }

            auto vecIt = std::find(_stackCardViews.begin(), _stackCardViews.end(), topCardView);
            if (vecIt != _stackCardViews.end()) {
                _stackCardViews.erase(vecIt);
            }

            topCardView->removeFromParent();
            topCardView->release();
        }

        if (success) {
            // 模型与交互已在动画开始前更新，这里仅做显示层的后续处理
            CCLOG("StackController - Hand cards remaining (after animation): %zu", _gameModel->getStackCards().size());
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
    // 底牌显示由GameController统一管理
    // 这里我们通过回调机制通知GameController更新
    // 由于我们没有直接的回调，这里暂时留空
    // 实际的底牌更新应该在动画完成时由调用方处理
    CCLOG("StackController::updateCurrentCardDisplay - Need external update");
}

bool StackController::recordUndoOperation(std::shared_ptr<CardModel> sourceCard, 
                                         std::shared_ptr<CardModel> targetCard) {
    if (!_undoManager || !sourceCard || !targetCard) {
        return false;
    }

    // 获取手牌堆顶部卡牌的实际显示位置（相对于GameView的绝对位置）
    Vec2 sourcePosition = Vec2::ZERO;
    Vec2 targetPosition = Vec2::ZERO;
    
    if (_configManager) {
        auto uiConfig = _configManager->getUILayoutConfig();
        if (uiConfig) {
            // 计算手牌堆顶部卡牌的绝对位置
            Vec2 stackPos = uiConfig->getStackPosition();
            float stackOffset = uiConfig->getStackCardOffset();
            
            // 假设这是栈顶卡牌，计算其在手牌堆中的位置
            const auto& stackCards = _gameModel->getStackCards();
            if (!stackCards.empty()) {
                size_t topIndex = stackCards.size() - 1;
                Vec2 relativePos = Vec2(topIndex * stackOffset, 0);
                sourcePosition = stackPos + relativePos;
            } else {
                sourcePosition = stackPos;
            }
            
            // 底牌区域的绝对位置
            targetPosition = uiConfig->getCurrentCardPosition();
        }
    }
    
    CCLOG("StackController::recordUndoOperation - Recording positions: source(%.0f,%.0f) -> target(%.0f,%.0f)",
          sourcePosition.x, sourcePosition.y, targetPosition.x, targetPosition.y);
    
    // 创建撤销记录
    auto undoModel = UndoModel::createStackToCurrentAction(
        sourceCard, targetCard,
        sourcePosition, targetPosition
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
    CCLOG("StackController::onStackCardClicked - ENTRY");
    CCLOG("  cardView: %p, cardModel: %p", cardView, cardModel.get());
    CCLOG("  _isInitialized: %s", _isInitialized ? "true" : "false");
    
    if (!cardView || !cardModel) {
        CCLOG("StackController::onStackCardClicked - Invalid parameters, exiting");
        return;
    }
    
    CCLOG("StackController::onStackCardClicked - Stack card clicked: %s", cardModel->toString().c_str());
    CCLOG("  CardView enabled state: %s", cardView->isEnabled() ? "ENABLED" : "DISABLED");
    
    // 检查卡牌是否被禁用
    if (!cardView->isEnabled()) {
        CCLOG("StackController::onStackCardClicked - Card is disabled, ignoring click");
        return;
    }
    
    // 只有顶部卡牌可以点击
    auto topCard = getTopCard();
    CCLOG("  Current top card: %s", topCard ? topCard->toString().c_str() : "null");
    
    if (!topCard || topCard->getCardId() != cardModel->getCardId()) {
        CCLOG("StackController::onStackCardClicked - Only top card can be clicked (clicked: %d, top: %d)", 
              cardModel->getCardId(), 
              topCard ? topCard->getCardId() : -1);
        return;
    }
    
    CCLOG("StackController::onStackCardClicked - Valid top card click, proceeding...");
    
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
    CCLOG("StackController::updateStackInteractivity - Starting");
    
    // 安全检查
    if (!_gameModel) {
        CCLOG("StackController::updateStackInteractivity - GameModel is null, aborting");
        return;
    }
    
    CCLOG("  _stackCardViews size: %zu", _stackCardViews.size());
    CCLOG("  _cardViewMap size: %zu", _cardViewMap.size());
    
    // 只有顶部卡牌可以交互
    auto topCard = getTopCard();
    CCLOG("  Top card from model: %s", topCard ? topCard->toString().c_str() : "null");
    
    for (auto cardView : _stackCardViews) {
        if (cardView && cardView->getCardModel()) {
            bool isTopCard = topCard && (cardView->getCardModel()->getCardId() == topCard->getCardId());
            cardView->setEnabled(isTopCard);
            CCLOG("    Card ID %d, View %p: %s", 
                  cardView->getCardModel()->getCardId(), 
                  cardView,
                  isTopCard ? "ENABLED" : "disabled");
        }
    }
    
    CCLOG("StackController::updateStackInteractivity - Complete");
}

void StackController::registerCardView(CardView* cardView) {
    if (!cardView || !cardView->getCardModel()) {
        CCLOG("StackController::registerCardView - Invalid card view");
        return;
    }
    
    int cardId = cardView->getCardModel()->getCardId();
    
    // 检查是否已经注册过
    auto existingIt = _cardViewMap.find(cardId);
    if (existingIt != _cardViewMap.end()) {
        CCLOG("StackController::registerCardView - Card %d already registered, updating view", cardId);
        
        // 从列表中移除旧视图
        auto listIt = std::find(_stackCardViews.begin(), _stackCardViews.end(), existingIt->second);
        if (listIt != _stackCardViews.end()) {
            _stackCardViews.erase(listIt);
        }
    }
    
    // 添加到映射
    _cardViewMap[cardId] = cardView;
    
    // 添加到列表
    _stackCardViews.push_back(cardView);
    
    // 设置点击回调
    cardView->setCardClickCallback([this](CardView* view, std::shared_ptr<CardModel> model) {
        onStackCardClicked(view, model);
    });
    
    // 更新交互性（确保只有栈顶卡牌可以点击）
    updateStackInteractivity();
    
    CCLOG("StackController::registerCardView - Registered card %s (ID: %d)", 
          cardView->getCardModel()->toString().c_str(), cardId);
}

bool StackController::initialDealCurrentFromStack() {
    CCLOG("StackController::initialDealCurrentFromStack - Starting initialization");
    CCLOG("  _initialDealt: %s, _isInitialized: %s", 
          _initialDealt ? "true" : "false", 
          _isInitialized ? "true" : "false");
    
    if (_initialDealt || !_isInitialized) {
        CCLOG("StackController::initialDealCurrentFromStack - Early return: already dealt or not initialized");
        return false;
    }

    // 若已有当前底牌（栈不为空）则不执行
    bool stackEmpty = _gameModel->isCurrentCardStackEmpty();
    CCLOG("StackController::initialDealCurrentFromStack - Stack empty: %s", stackEmpty ? "true" : "false");
    
    if (!stackEmpty) {
        CCLOG("StackController::initialDealCurrentFromStack - Stack not empty, skipping initialization");
        const auto& stack = _gameModel->getCurrentCardStack();
        CCLOG("  Current stack size: %zu", stack.size());
        for (size_t i = 0; i < stack.size(); i++) {
            CCLOG("    Stack[%zu]: %s", i, stack[i]->toString().c_str());
        }
        return false;
    }

    auto topCard = getTopCard();
    auto topCardView = getTopCardView();
    
    CCLOG("StackController::initialDealCurrentFromStack - Top card: %s", 
          topCard ? topCard->toString().c_str() : "null");
    CCLOG("StackController::initialDealCurrentFromStack - Top card view: %p", topCardView);
    
    if (!topCard || !topCardView) {
        CCLOG("StackController::initialDealCurrentFromStack - No top card or view available, aborting");
        return false;
    }

    // 使用栈结构设置为当前底牌（不记录撤销）
    _gameModel->pushCurrentCard(topCard);
    
    // 从手牌栈中移除这张卡牌
    _gameModel->removeTopStackCard();
    CCLOG("StackController::initialDealCurrentFromStack - Removed card from stack, pushed to current card stack");
    
    // 验证栈状态
    auto newTopCard = getTopCard();
    CCLOG("  New top card after removal: %s", newTopCard ? newTopCard->toString().c_str() : "null");
    CCLOG("  Stack size after removal: %zu", _gameModel->getStackCards().size());

    auto uiLayoutConfig = _configManager->getUILayoutConfig();

    // 计算动画坐标（提升到GameView层）
    Node* srcParent = topCardView->getParent();
    Node* overlayParent = (srcParent && srcParent->getParent()) ? srcParent->getParent() : srcParent;
    
    CCLOG("StackController::initialDealCurrentFromStack - Animation setup:");
    CCLOG("  srcParent: %p, overlayParent: %p", srcParent, overlayParent);
    
    Vec2 worldStart = srcParent->convertToWorldSpace(topCardView->getPosition());
    Vec2 worldTarget = uiLayoutConfig->getCurrentCardPosition();
    Vec2 startInOverlay = overlayParent->convertToNodeSpace(worldStart);
    Vec2 targetInOverlay = overlayParent->convertToNodeSpace(worldTarget);

    CCLOG("  worldStart: (%.2f, %.2f), worldTarget: (%.2f, %.2f)", 
          worldStart.x, worldStart.y, worldTarget.x, worldTarget.y);
    CCLOG("  startInOverlay: (%.2f, %.2f), targetInOverlay: (%.2f, %.2f)", 
          startInOverlay.x, startInOverlay.y, targetInOverlay.x, targetInOverlay.y);

    // 提升层级，播放动画
    topCardView->retain();
    topCardView->removeFromParent();
    overlayParent->addChild(topCardView, 500); // 动画层
    topCardView->setPosition(startInOverlay);
    topCardView->setEnabled(false);

    CCLOG("StackController::initialDealCurrentFromStack - Starting animation from (%.2f, %.2f) to (%.2f, %.2f)", 
          startInOverlay.x, startInOverlay.y, targetInOverlay.x, targetInOverlay.y);

    int topCardId = topCard->getCardId();

    playMoveAnimation(topCardView, targetInOverlay, [this, topCardView, topCardId, overlayParent](bool success){
        CCLOG("StackController::initialDealCurrentFromStack - Animation callback called, success: %s", 
              success ? "true" : "false");
        
        // 动画完成，卡牌已经在正确位置，需要保存位置并重新添加到父节点
        if (success) {
            topCardView->retain();
            
            // 保存当前位置（动画已经移动到的正确位置）
            Vec2 currentPosition = topCardView->getPosition();
            CCLOG("  Final card position: (%.2f, %.2f)", currentPosition.x, currentPosition.y);
            
            topCardView->removeFromParent();
            
            // 重新添加到 GameView 的 _currentCardArea
            // 需要找到 currentCardArea 节点
            Node* currentCardArea = overlayParent->getChildByName("currentCardArea");
            if (currentCardArea) {
                // 转换坐标到 currentCardArea 的本地坐标系
                Vec2 localPos = currentCardArea->convertToNodeSpace(currentPosition);
                currentCardArea->addChild(topCardView, 300); // 当前底牌层
                topCardView->setPosition(localPos);
            } else {
                // 备用方案：直接添加到 GameView
                overlayParent->addChild(topCardView, 300);
                topCardView->setPosition(currentPosition);
            }
            
            topCardView->setEnabled(false);
            // 注意：不再在StackController中维护_currentCardView引用，避免悬挂指针
            
            // 从映射与列表移除，避免重复显示
            _cardViewMap.erase(topCardId);
            auto it = std::find(_stackCardViews.begin(), _stackCardViews.end(), topCardView);
            if (it != _stackCardViews.end()) _stackCardViews.erase(it);
            topCardView->release();
            
            // 立即更新交互状态，让新的栈顶卡牌可点击
            CCLOG("StackController::initialDealCurrentFromStack - About to update stack interactivity");
            CCLOG("  _stackCardViews size after removal: %zu", _stackCardViews.size());
            CCLOG("  _cardViewMap size after removal: %zu", _cardViewMap.size());
            updateStackInteractivity();
        } else {
            CCLOG("  Animation failed, removing card");
            topCardView->removeFromParent();
        }
        topCardView->release();

        // 更新交互 & 翻开下一张
        revealNextCard();
        updateStackInteractivity();
        updateCurrentCardDisplay();
        
        CCLOG("StackController::initialDealCurrentFromStack - Initialization complete");
    });

    _initialDealt = true;
    return true;
}
