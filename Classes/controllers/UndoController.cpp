#include "UndoController.h"

UndoController::UndoController()
    : _gameView(nullptr)
    , _gameModel(nullptr)
    , _undoManager(nullptr)
    , _playfieldController(nullptr)
    , _stackController(nullptr)
    , _isInitialized(false) {
}

UndoController::~UndoController() {
    // 注意：这里不需要删除引用的对象，它们由GameController管理
}

bool UndoController::init(GameView* gameView, 
                         std::shared_ptr<GameModel> gameModel,
                         UndoManager* undoManager,
                         PlayFieldController* playfieldController,
                         StackController* stackController) {
    if (!gameView || !gameModel || !undoManager || !playfieldController || !stackController) {
        CCLOG("UndoController::init - Invalid parameters");
        return false;
    }
    
    _gameView = gameView;
    _gameModel = gameModel;
    _undoManager = undoManager;
    _playfieldController = playfieldController;
    _stackController = stackController;
    
    _isInitialized = true;
    // undo controller initialized
    
    return true;
}

bool UndoController::performUndo() {
    // undo requested
    
    if (!_isInitialized || !_undoManager || !_gameModel) {
        CCLOG("UndoController::performUndo - Controller not initialized properly");
        return false;
    }
    
    if (!_undoManager->canUndo()) {
        CCLOG("UndoController::performUndo - No undo operations available");
        return false;
    }
    
    // current undo count, performing
    
    // 执行撤销操作（完全保持原有逻辑）
    bool success = _undoManager->performUndo([this](bool undoSuccess, std::shared_ptr<UndoModel> undoModel) {
        if (undoSuccess && undoModel) {
            // undo ok
            
            // 执行回退动画
            this->performUndoAnimation(undoModel);
            
            // 更新显示
            this->updateGameDisplay();
        } else {
            CCLOG("UndoController::performUndo - Undo failed");
        }
    });
    
    return success;
}

void UndoController::performUndoAnimation(std::shared_ptr<UndoModel> undoModel) {
    if (!undoModel || !_gameView) {
        CCLOG("UndoController::performUndoAnimation - Invalid parameters");
        return;
    }
    
    // start undo animation
    
    // 根据撤销操作类型执行相应的动画（保持原有逻辑）
    auto sourceCard = undoModel->getSourceCard();
    auto targetCard = undoModel->getTargetCard();
    
    if (!sourceCard) {
        CCLOG("UndoController::performUndoAnimation - No source card in undo model");
        return;
    }
    
    // 检查撤销操作类型
    UndoOperationType opType = undoModel->getOperationType();
    
    if (opType == UndoOperationType::CARD_MOVE) {
        // 桌面牌撤销：需要重新创建卡牌视图并放回桌面区域
        performPlayfieldCardUndoAnimation(undoModel);
    } else if (opType == UndoOperationType::STACK_OPERATION) {
        // 手牌堆撤销：需要重新创建卡牌视图并放回手牌堆
        performStackCardUndoAnimation(undoModel);
    }
}

void UndoController::performPlayfieldCardUndoAnimation(std::shared_ptr<UndoModel> undoModel) {
    auto sourceCard = undoModel->getSourceCard();
    
    // 获取当前底牌视图（即将移动回桌面的卡牌）
    auto currentCardView = _gameView->getCurrentCardView();
    if (!currentCardView) {
        CCLOG("UndoController::performPlayfieldCardUndoAnimation - No current card view found");
        return;
    }
    
    // === 关键调试信息 ===（保持原有的调试逻辑）
    // debug start
    
    if (currentCardView) {
        auto cardModel = currentCardView->getCardModel();
        if (cardModel) {
            // debug current card
        } else {
            // no model
        }
    }
    // debug end
    
    // 强制确保CardView显示正确的卡牌内容（保持原有逻辑）
    currentCardView->setCardModel(sourceCard);
    currentCardView->updateDisplay();
    currentCardView->setFlipped(true, false); // 强制设置为正面显示
    // forced update
    
    // 使用记录的世界坐标作为目标位置（保持原有逻辑）
    Vec2 worldTargetPos = undoModel->getSourcePosition(); // 这现在是正确的世界坐标
    
    // target pos
    
    // 使用与现有动画完全相同的坐标转换方式（保持原有逻辑）
    Node* currentParent = currentCardView->getParent(); // 底牌区域
    Node* gameViewNode = currentParent->getParent();    // GameView（作为动画层）
    
    // 计算动画坐标（完全复制PlayFieldController的逻辑）
    Vec2 worldStart = currentParent->convertToWorldSpace(currentCardView->getPosition());
    Vec2 worldTarget = worldTargetPos; // 使用记录的世界坐标
    
    Vec2 startInGameView = gameViewNode->convertToNodeSpace(worldStart);
    Vec2 targetInGameView = gameViewNode->convertToNodeSpace(worldTarget);
    
    // pos conversion
    
    // 提升层级进行动画（与PlayFieldController完全一致）
    currentCardView->retain();
    currentCardView->removeFromParent();
    gameViewNode->addChild(currentCardView, 500); // 动画层
    currentCardView->setPosition(startInGameView);
    currentCardView->setEnabled(false);
    
    // 立即更新底牌显示（在动画开始前，避免视觉冲突）
    this->updateCurrentCardDisplay();
    // updated current card display
    
    // 播放回退动画（保持原有逻辑）
    int originalZOrder = undoModel->getSourceZOrder();
    _gameView->playCardMoveAnimation(currentCardView, targetInGameView, 0.5f, [this, currentCardView, sourceCard, worldTargetPos, originalZOrder]() {
        // animation completed
        
        // 动画完成后，恢复卡牌到桌面区域
        // 恢复到桌面时带回原始z序
        this->restoreCardToPlayfield(currentCardView, sourceCard, worldTargetPos, originalZOrder);
        
        // 注意：不在这里更新底牌显示，避免与动画产生视觉冲突
        // 底牌显示应该在动画开始前或通过其他机制独立更新
    });
}

void UndoController::restoreCardToPlayfield(CardView* cardView, std::shared_ptr<CardModel> cardModel, const Vec2& absolutePos, int originalZOrder) {
    if (!cardView || !_playfieldController) {
        CCLOG("UndoController::restoreCardToPlayfield - Invalid parameters");
        return;
    }
    
    // 获取桌面区域节点并计算正确的相对坐标（保持原有逻辑）
    auto playfieldArea = _gameView->getPlayfieldArea();
    if (!playfieldArea) {
        CCLOG("UndoController::restoreCardToPlayfield - Playfield area not found");
        cardView->release();
        return;
    }
    
    // 将世界坐标转换为桌面区域的本地坐标（正确的坐标转换）
    Vec2 relativePos = playfieldArea->convertToNodeSpace(absolutePos);
    
    // restoring card to playfield
    
    // 更新卡牌模型位置（保持原有逻辑）
    cardModel->setPosition(relativePos);
    
    // 将卡牌重新添加到桌面区域（保持原有逻辑）
    cardView->retain();
    cardView->removeFromParent();
    // 使用与初始生成一致的层级（默认0），避免回退后层级过高
    playfieldArea->addChild(cardView);
    // 恢复原始本地z序
    cardView->setLocalZOrder(originalZOrder);
    // applied local z
    cardView->setPosition(relativePos);
    cardView->setEnabled(true); // 重新启用交互
    
    // 重新注册到GameView（保持原有逻辑）
    auto& playfieldViews = const_cast<std::vector<CardView*>&>(_gameView->getPlayfieldCardViews());
    playfieldViews.push_back(cardView);
    
    auto& cardViewMap = const_cast<std::map<int, CardView*>&>(_gameView->getCardViewMap());
    cardViewMap[cardModel->getCardId()] = cardView;
    
    // 关键修复：注册到PlayFieldController（这会设置正确的点击回调）
    if (_playfieldController) {
        _playfieldController->registerCardView(cardView);
    }
    
    // restored
    cardView->release();
}

void UndoController::updateCurrentCardDisplay() {
    if (!_gameModel || !_gameView) {
        CCLOG("UndoController::updateCurrentCardDisplay - Invalid game state");
        return;
    }
    
    // 获取当前底牌（保持原有逻辑）
    auto currentCard = _gameModel->getCurrentCard();
    if (!currentCard) {
        CCLOG("UndoController::updateCurrentCardDisplay - No current card in model");
        // 底牌区域置空，同时重置所有控制器的_currentCardView指针
        _gameView->setCurrentCardView(nullptr);
        if (_playfieldController) {
            _playfieldController->setCurrentCardView(nullptr);
        }
        if (_stackController) {
            _stackController->setCurrentCardView(nullptr);
        }
        return;
    }
    
    // updating current card
    
    // 创建新的底牌视图（保持原有逻辑）
    auto newCurrentCardView = CardView::create(currentCard);
    if (!newCurrentCardView) {
        CCLOG("UndoController::updateCurrentCardDisplay - Failed to create card view");
        return;
    }
    
    // 获取底牌区域（保持原有逻辑）
    auto currentCardArea = _gameView->getCurrentCardArea();
    if (!currentCardArea) {
        CCLOG("UndoController::updateCurrentCardDisplay - No current card area");
        return;
    }
    
    // 关键修复：先将所有控制器的_currentCardView设置为nullptr，避免悬挂指针
    _gameView->setCurrentCardView(nullptr);
    if (_playfieldController) {
        _playfieldController->setCurrentCardView(nullptr);
    }
    if (_stackController) {
        _stackController->setCurrentCardView(nullptr);
    }
    
    // 清除旧的底牌视图（保持原有逻辑）
    currentCardArea->removeAllChildren();
    
    // 添加新的底牌视图
    currentCardArea->addChild(newCurrentCardView, 300);
    newCurrentCardView->setPosition(Vec2(0, 0)); // 居中显示
    newCurrentCardView->setFlipped(true, false);  // 正面显示，无动画
    newCurrentCardView->setEnabled(false);        // 底牌不可点击
    
    // 同步更新所有引用
    _gameView->setCurrentCardView(newCurrentCardView);
    if (_playfieldController) {
        _playfieldController->setCurrentCardView(newCurrentCardView);
    }
    if (_stackController) {
        _stackController->setCurrentCardView(newCurrentCardView);
    }
    
    // updated
}

void UndoController::performStackCardUndoAnimation(std::shared_ptr<UndoModel> undoModel) {
    auto sourceCard = undoModel->getSourceCard();
    
    // creating view for card
    
    // 为要回退的卡牌创建新的视图（因为模型已经更新，当前底牌视图已经是恢复后的底牌）
    auto cardViewToAnimate = CardView::create(sourceCard);
    if (!cardViewToAnimate) {
        CCLOG("UndoController::performStackCardUndoAnimation - Failed to create card view");
        return;
    }
    
    // 目标位置（手牌堆的绝对位置）（保持原有逻辑）
    Vec2 worldTargetPos = undoModel->getSourcePosition();
    
    // target pos
    
    // 获取底牌区域作为起始位置（保持原有逻辑）
    auto currentCardArea = _gameView->getCurrentCardArea();
    if (!currentCardArea) {
        CCLOG("UndoController::performStackCardUndoAnimation - No current card area found");
        return;
    }
    
    // 使用与桌面牌回退相同的坐标转换方式（保持原有逻辑）
    Node* gameViewNode = currentCardArea->getParent();    // GameView（作为动画层）
    
    // 将新创建的卡牌视图放在底牌区域的中心作为动画起点（保持原有逻辑）
    Vec2 worldStart = currentCardArea->convertToWorldSpace(Vec2(0, 0)); // 底牌区域中心
    Vec2 worldTarget = worldTargetPos; // 使用记录的世界坐标
    
    Vec2 startInGameView = gameViewNode->convertToNodeSpace(worldStart);
    Vec2 targetInGameView = gameViewNode->convertToNodeSpace(worldTarget);
    
    // pos conversion
    
    // 设置动画卡牌的初始状态和位置（保持原有逻辑）
    cardViewToAnimate->setFlipped(true, false); // 正面显示
    cardViewToAnimate->setEnabled(false); // 动画期间不可点击
    
    // 将卡牌添加到动画层（保持原有逻辑）
    gameViewNode->addChild(cardViewToAnimate, 500); // 动画层
    cardViewToAnimate->setPosition(startInGameView);
    
    // 立即更新底牌显示（在动画开始前，避免视觉冲突）
    this->updateCurrentCardDisplay();
    // updated current card display
    
    // 播放回退动画（保持原有逻辑）
    _gameView->playCardMoveAnimation(cardViewToAnimate, targetInGameView, 0.5f, [this, cardViewToAnimate, worldTargetPos, sourceCard]() {
        // animation completed
        
        // 动画完成后，将卡牌重新添加到手牌堆
        this->restoreCardToStack(cardViewToAnimate, sourceCard, worldTargetPos);
    });
}

void UndoController::restoreCardToStack(CardView* cardView, std::shared_ptr<CardModel> cardModel, const Vec2& absolutePos) {
    if (!cardView || !_stackController) {
        CCLOG("UndoController::restoreCardToStack - Invalid parameters");
        return;
    }
    
    // 获取手牌堆区域节点并计算正确的相对坐标（保持原有逻辑）
    auto stackArea = _gameView->getStackArea();
    if (!stackArea) {
        CCLOG("UndoController::restoreCardToStack - Stack area not found");
        cardView->release();
        return;
    }
    
    // 将世界坐标转换为手牌堆区域的本地坐标（保持原有逻辑）
    Vec2 relativePos = stackArea->convertToNodeSpace(absolutePos);
    
    // restoring card to stack
    
    // 更新卡牌模型位置（保持原有逻辑）
    cardModel->setPosition(relativePos);
    
    // 将卡牌重新添加到手牌堆区域（保持原有逻辑）
    cardView->retain();
    cardView->removeFromParent();
    stackArea->addChild(cardView, 100);
    cardView->setPosition(relativePos);
    cardView->setEnabled(true); // 重新启用交互（作为栈顶卡牌）
    
    // 重新注册到GameView（保持原有逻辑）
    auto& stackViews = const_cast<std::vector<CardView*>&>(_gameView->getStackCardViews());
    stackViews.push_back(cardView);
    
    auto& cardViewMap = const_cast<std::map<int, CardView*>&>(_gameView->getCardViewMap());
    cardViewMap[cardModel->getCardId()] = cardView;
    
    // 关键修复：注册到StackController（保持原有逻辑）
    if (_stackController) {
        _stackController->registerCardView(cardView);
    }
    
    // restored
    cardView->release();
}

void UndoController::updateGameDisplay() {
    if (_gameView && _gameModel) {
        _gameView->updateDisplay(_gameModel);
    }
}
