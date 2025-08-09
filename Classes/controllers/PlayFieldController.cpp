#include "PlayFieldController.h"
#include "../views/GameView.h"
#include <algorithm>

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
    
    
    
    return true;
}

bool PlayFieldController::handleCardClick(int cardId, const CardClickCallback& callback) {
    if (!_isInitialized) {
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
    
    
    
    // 检查卡片是否满足移动条件
    if (!checkMoveConditions(cardModel)) {
        if (callback) callback(false, cardModel);
        return false;
    }
    
    // 执行替换操作（允许并发）
    return replaceTrayWithPlayFieldCard(cardId, [this, callback, cardModel](bool success) {
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
    
    // 1. 记录撤销操作
    auto currentCard = _gameModel->getCurrentCard();
    if (!recordUndoOperation(cardModel, currentCard)) {
        CCLOG("PlayFieldController::replaceTrayWithPlayFieldCard - Failed to record undo");
        if (callback) callback(false);
        return false;
    }
    
    // 2. 更新model数据 - 使用栈结构
    _gameModel->pushCurrentCard(cardModel);
    
    // 打印栈信息
    const auto& stack = _gameModel->getCurrentCardStack();
    // stack size and current card (verbose logs removed)
    
    // 3. 执行动画并替换底牌显示
    auto uiLayoutConfig = _configManager->getUILayoutConfig();

    cocos2d::Node* srcParent = cardView->getParent();
    cocos2d::Node* dstParent = _currentCardView ? _currentCardView->getParent() : nullptr; // 当前底牌区域
    cocos2d::Node* overlayParent = (dstParent && dstParent->getParent()) ? dstParent->getParent() : (srcParent && srcParent->getParent() ? srcParent->getParent() : srcParent);

    cocos2d::Vec2 worldStart = srcParent->convertToWorldSpace(cardView->getPosition());
    cocos2d::Vec2 worldTarget = dstParent
        ? dstParent->convertToWorldSpace(_currentCardView->getPosition())
        : uiLayoutConfig->getCurrentCardPosition();

    cocos2d::Vec2 startInOverlay = overlayParent->convertToNodeSpace(worldStart);
    cocos2d::Vec2 targetInOverlay = overlayParent->convertToNodeSpace(worldTarget);

    // 提升层级并移动
    cardView->retain();
    cardView->removeFromParent();
    overlayParent->addChild(cardView, 500); // 动画层
    cardView->setPosition(startInOverlay);
    cardView->setEnabled(false);

    int movedCardId = cardModel->getCardId();

    playMoveAnimation(cardView, targetInOverlay, [this, callback, movedCardId, cardView, overlayParent](bool success) {
        // 动画结束后，替换底牌显示
        if (success) {
            // 如果有底牌区域，直接替换显示
            if (_currentCardArea) {
                // 移除旧的底牌视图
                if (_currentCardView && _currentCardView != cardView) {
                    _currentCardView->removeFromParent();
                } else {
                    // 清除底牌区域中的所有子视图
                    _currentCardArea->removeAllChildren();
                }
                
                // 将新卡牌移入底牌区域
                cardView->retain();
                cardView->removeFromParent();
                _currentCardArea->addChild(cardView, 300); // 当前底牌层
                
                // 设置卡牌锚点为中心，然后放在区域中心
                cardView->setAnchorPoint(Vec2(0.5f, 0.5f));
                // 由于容器使用默认锚点(0,0)，卡牌使用中心锚点(0.5,0.5)
                // 卡牌放在(0,0)位置就能在容器中心显示
                Vec2 centerPos = Vec2(0, 0);
                cardView->setPosition(centerPos);
                
                cardView->setVisible(true);
                cardView->release();
                
                // 强制设置为正面显示
                cardView->setFlipped(true, false);
                
                // 注意：不需要调用setCardModel()
                // cardView已经绑定了正确的CardModel，重新设置可能导致显示错乱
            } else {
                // 如果没有底牌区域，直接在overlay中替换
                if (_currentCardView && _currentCardView != cardView) {
                    _currentCardView->removeFromParent();
                }
                
                auto uiLayoutConfig = _configManager->getUILayoutConfig();
                Vec2 worldTarget = uiLayoutConfig->getCurrentCardPosition();
                Vec2 targetInOverlay = overlayParent->convertToNodeSpace(worldTarget);
                
                cardView->setLocalZOrder(300); // 当前底牌层
                cardView->setPosition(targetInOverlay);
                cardView->setVisible(true);
            }
            cardView->setEnabled(false); // 当前底牌不可点击

            // 更新引用
            _currentCardView = cardView;
            
            // 同步更新GameView的_currentCardView引用
            if (_gameView) {
                _gameView->setCurrentCardView(cardView);
            }

            // 从映射和列表中移除该卡视图，避免重复显示
            _cardViewMap.erase(movedCardId);
            auto it = std::find(_playfieldCardViews.begin(), _playfieldCardViews.end(), cardView);
            if (it != _playfieldCardViews.end()) _playfieldCardViews.erase(it);
            
            // 关键修复：从GameModel的桌面卡牌列表中移除
            _gameModel->removePlayfieldCard(movedCardId);
            CCLOG("PlayFieldController::replaceTrayWithPlayFieldCard - Removed card from playfield model: %d", movedCardId);
            
            // 注意：不调用cardView->updateDisplay()，因为它会重置位置为model中的位置
        } else {
            // 失败时释放临时引用
            cardView->removeFromParent();
        }

        // 释放 retain
        cardView->release();

        if (callback) callback(success);
    });
    
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

void PlayFieldController::registerCardView(CardView* cardView) {
    if (!cardView || !cardView->getCardModel()) {
        CCLOG("PlayFieldController::registerCardView - Invalid card view");
        return;
    }
    
    int cardId = cardView->getCardModel()->getCardId();
    
    // 添加到映射
    _cardViewMap[cardId] = cardView;
    
    // 添加到列表
    _playfieldCardViews.push_back(cardView);
    
    // 设置点击回调
    cardView->setCardClickCallback([this](CardView* view, std::shared_ptr<CardModel> model) {
        onCardClicked(view, model);
    });
    
    CCLOG("PlayFieldController::registerCardView - Registered card %s (ID: %d)", 
          cardView->getCardModel()->toString().c_str(), cardId);
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

    // 获取卡牌视图以进行正确的坐标转换
    auto cardView = getCardView(sourceCard->getCardId());
    if (!cardView) {
        CCLOG("PlayFieldController::recordUndoOperation - Card view not found for card ID: %d", sourceCard->getCardId());
        return false;
    }

    // 使用与正常动画相同的坐标转换方式获取世界坐标
    Node* srcParent = cardView->getParent(); // 桌面区域
    Vec2 sourcePosition = srcParent->convertToWorldSpace(cardView->getPosition());
    int sourceZOrder = cardView->getLocalZOrder();
    
    // 获取底牌的世界坐标
    Vec2 targetPosition = Vec2::ZERO;
    if (_currentCardView && _currentCardView->getParent()) {
        // 使用现有底牌的世界坐标（与正常动画一致）
        targetPosition = _currentCardView->getParent()->convertToWorldSpace(_currentCardView->getPosition());
    } else {
        // 备用方案：使用配置的底牌位置（这应该已经是世界坐标）
        targetPosition = _configManager->getUILayoutConfig()->getCurrentCardPosition();
    }
    
    
    
    // 创建撤销记录
    auto undoModel = UndoModel::createPlayfieldToCurrentAction(
        sourceCard, targetCard,
        sourcePosition, targetPosition,
        0,
        sourceZOrder
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
    
    
    
    // 处理点击事件
    handleCardClick(cardModel->getCardId(), [this](bool success, std::shared_ptr<CardModel> card) {
        if (success) {
            // 通知外部回调
            if (_cardClickCallback) {
                _cardClickCallback(true, card);
            }
        } else {
            if (_cardClickCallback) {
                _cardClickCallback(false, card);
            }
        }
    });
}
