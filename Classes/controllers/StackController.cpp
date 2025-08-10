#include "StackController.h"
#include <algorithm>

StackController::StackController()
    : BaseController()
    , _currentCardView(nullptr)
    , _isInitialized(false)
    , _isProcessingOperation(false)
    , _initialDealt(false) {
}

StackController::~StackController() {
}

bool StackController::init(std::shared_ptr<GameModel> gameModel, UndoManager* undoManager) {
    // 调用基类初始化
    if (!initBase(gameModel, undoManager)) {
        CCLOG("StackController::init - Base initialization failed");
        return false;
    }

    _isInitialized = true;

    
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
    
    
    
    // 执行替换操作（允许并发）
    return replaceCurrentWithTopCard([this, callback, topCard](bool success) {
        if (callback) callback(success, topCard);
    });
}

bool StackController::replaceCurrentWithTopCard(const AnimationCallback& callback) {
    if (!_isInitialized) {
        if (callback) callback(false);
        return false;
    }
    
    auto topCard = getTopCard();
    auto topCardView = getTopCardView();
    
    if (!topCard || !topCardView) {
        if (callback) callback(false);
        return false;
    }
    
    // 1. 记录撤销操作
    auto currentCard = _gameModel->getCurrentCard();
    
    // 获取源卡牌位置信息
    Vec2 sourcePosition = getWorldPosition(topCardView);
    int sourceZOrder = topCardView->getLocalZOrder();
    
    // 获取目标位置
    auto uiLayoutConfig = _configManager->getUILayoutConfig();
    Vec2 targetPosition = uiLayoutConfig->getCurrentCardPosition();
    
    // 计算手牌堆中的索引
    const auto& stackCards = _gameModel->getStackCards();
    int sourceStackIndex = stackCards.empty() ? 0 : static_cast<int>(stackCards.size() - 1);
    
    // 使用BaseController的记录方法
    if (!recordUndoOperationBase(topCard, currentCard, sourcePosition, targetPosition, 
                                sourceStackIndex, sourceZOrder, UndoOperationType::STACK_OPERATION)) {
        if (callback) callback(false);
        return false;
    }
    
    // 2. 更新model数据 - 先添加到底牌栈（但不从手牌栈移除）
    _gameModel->pushCurrentCard(topCard);
    
    // 栈信息（调试日志已移除）
    
    // 3. 执行动画：顶部手牌移动到配置的底牌位置
    Vec2 targetWorldPosition = targetPosition; // 重用之前计算的目标位置

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

    // 使用BaseController的通用动画方法
    moveCardWithAnimation(topCardView, targetWorldPosition, 500, [this, callback, topCardId, topCardView](bool success) {
        // 安全检查：确保对象仍然有效
        if (!this || !_gameModel) {
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
            // 模型与交互已在动画开始前更新
            updateCurrentCardDisplay();
        }

        if (callback) callback(success);
    });
    
    
    
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
    
}

void StackController::onStackCardClicked(CardView* cardView, std::shared_ptr<CardModel> cardModel) {
    if (!cardView || !cardModel) {
        return;
    }
    
    // 检查卡牌是否被禁用
    if (!cardView->isEnabled()) {
        return;
    }
    
    // 只有顶部卡牌可以点击
    auto topCard = getTopCard();
    if (!topCard || topCard->getCardId() != cardModel->getCardId()) {
        return;
    }
    
    // 处理顶部卡牌点击
    handleTopCardClick([this](bool success, std::shared_ptr<CardModel> card) {
        if (success) {
            // 通知外部回调
            if (_stackOperationCallback) {
                _stackOperationCallback(true, card);
            }
        } else {
            if (_stackOperationCallback) {
                _stackOperationCallback(false, card);
            }
        }
    });
}

void StackController::updateStackInteractivity() {
    // 安全检查
    if (!_gameModel) {
        return;
    }
    
    // 只有顶部卡牌可以交互
    auto topCard = getTopCard();
    
    for (auto cardView : _stackCardViews) {
        if (cardView && cardView->getCardModel()) {
            bool isTopCard = topCard && (cardView->getCardModel()->getCardId() == topCard->getCardId());
            cardView->setEnabled(isTopCard);
        }
    }
}

void StackController::registerCardView(CardView* cardView) {
    if (!cardView || !cardView->getCardModel()) {
        return;
    }
    
    int cardId = cardView->getCardModel()->getCardId();
    
    // 检查是否已经注册过
    auto existingIt = _cardViewMap.find(cardId);
    if (existingIt != _cardViewMap.end()) {
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
}

bool StackController::initialDealCurrentFromStack() {
    if (_initialDealt || !_isInitialized) {
        return false;
    }

    // 若已有当前底牌（栈不为空）则不执行
    bool stackEmpty = _gameModel->isCurrentCardStackEmpty();
    if (!stackEmpty) {
        return false;
    }

    auto topCard = getTopCard();
    auto topCardView = getTopCardView();
    
    if (!topCard || !topCardView) {
        return false;
    }

    // 使用栈结构设置为当前底牌（不记录撤销）
    _gameModel->pushCurrentCard(topCard);
    
    // 从手牌栈中移除这张卡牌
    _gameModel->removeTopStackCard();

    auto uiLayoutConfig = _configManager->getUILayoutConfig();
    Vec2 targetWorldPosition = uiLayoutConfig->getCurrentCardPosition();

    // 计算动画坐标（提升到GameView层） - 重构前的逻辑
    Node* srcParent = topCardView->getParent();
    Node* overlayParent = (srcParent && srcParent->getParent()) ? srcParent->getParent() : srcParent;

    int topCardId = topCard->getCardId();

    // 使用BaseController的通用动画方法
    moveCardWithAnimation(topCardView, targetWorldPosition, 500, [this, topCardView, topCardId, overlayParent](bool success){
        
        // 动画完成，卡牌已经在正确位置
        if (success) {
            // 动画完成，卡牌已经在正确位置，需要保存位置并重新添加到父节点
            topCardView->retain();
            
            // 保存当前位置（动画已经移动到的正确位置）
            Vec2 currentPosition = topCardView->getPosition();
            
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
            topCardView->release();
            
            // 从映射与列表移除，避免重复显示
            _cardViewMap.erase(topCardId);
            auto it = std::find(_stackCardViews.begin(), _stackCardViews.end(), topCardView);
            if (it != _stackCardViews.end()) _stackCardViews.erase(it);
            
            // 立即更新交互状态，让新的栈顶卡牌可点击
            updateStackInteractivity();
        } else {
            topCardView->removeFromParent();
        }

        // 更新交互 & 翻开下一张
        revealNextCard();
        updateStackInteractivity();
        updateCurrentCardDisplay();
    });

    _initialDealt = true;
    return true;
}
