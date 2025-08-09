#include "GameController.h"

GameController::GameController()
    : _gameView(nullptr)
    , _gameModel(nullptr)
    , _levelConfig(nullptr)
    , _playfieldController(nullptr)
    , _stackController(nullptr)
    , _undoManager(nullptr)
    , _currentLevelId(0)
    , _isInitialized(false) {
}

GameController::~GameController() {
    // 清理子控制器
    if (_playfieldController) {
        delete _playfieldController;
        _playfieldController = nullptr;
    }

    if (_stackController) {
        delete _stackController;
        _stackController = nullptr;
    }

    if (_undoManager) {
        delete _undoManager;
        _undoManager = nullptr;
    }
}

bool GameController::init(GameView* gameView) {
    if (!gameView) {
        CCLOG("GameController::init - Invalid game view");
        return false;
    }
    
    _gameView = gameView;
    _gameModel = std::make_shared<GameModel>();

    // 创建子控制器
    _playfieldController = new PlayFieldController();
    _stackController = new StackController();
    _undoManager = new UndoManager();

    _isInitialized = true;
    // controller initialized

    return true;
}

bool GameController::startGame(int levelId) {
    if (!_isInitialized) {
        CCLOG("GameController::startGame - Controller not initialized");
        return false;
    }

    // starting level

    // 按照README要求的初始化流程：

    // 1. 调用LevelConfigLoader::loadLevelConfig(levelId)获取LevelConfig
    _levelConfig = _configLoader.loadLevelConfig(levelId);
    if (!_levelConfig) {
        CCLOG("GameController::startGame - Failed to load level config for level %d", levelId);

        // 配置文件缺失，无法继续
        return false;
    }

    _currentLevelId = levelId;

    // 2. 使用GameModelFromLevelGenerator::generateGameModel生成GameModel
    _gameModel = GameModelFromLevelGenerator::generateGameModel(_levelConfig);
    if (!_gameModel) {
        CCLOG("GameController::startGame - Failed to generate game model");
        return false;
    }

    // 3. 初始化各子控制器
    if (!initializeSubControllers()) {
        CCLOG("GameController::startGame - Failed to initialize sub controllers");
        return false;
    }

    // 4. 创建GameView并添加到父节点（已在init中创建）
    if (!initializeGameView()) {
        CCLOG("GameController::startGame - Failed to initialize game view");
        return false;
    }

    // 5. 初始化各子控制器的视图
    if (!initializeSubControllerViews()) {
        CCLOG("GameController::startGame - Failed to initialize sub controller views");
        return false;
    }

    // 设置游戏状态为进行中
    _gameModel->setGameState(GameState::PLAYING);

    // game started

    // 新增：开场时发一张备用牌到当前底牌（带动画）
    if (_stackController) {
        _stackController->initialDealCurrentFromStack();
    }

    return true;
}

bool GameController::restartGame() {
    if (_currentLevelId <= 0) {
        CCLOG("GameController::restartGame - No current level to restart");
        return false;
    }
    
    return startGame(_currentLevelId);
}

void GameController::pauseGame() {
    if (_gameModel) {
        _gameModel->setGameState(GameState::PAUSED);
        // paused
    }
}

void GameController::resumeGame() {
    if (_gameModel) {
        _gameModel->setGameState(GameState::PLAYING);
        // resumed
    }
}

GameState GameController::getCurrentGameState() const {
    return _gameModel ? _gameModel->getGameState() : GameState::INITIALIZING;
}

int GameController::getCurrentLevelId() const {
    return _currentLevelId;
}

bool GameController::initializeSubControllers() {
    if (!_gameModel || !_playfieldController || !_stackController || !_undoManager) {
        CCLOG("GameController::initializeSubControllers - Missing components");
        return false;
    }

    // 按照README要求初始化各子控制器：

    // UndoManager::init(...)
    if (!_undoManager->init(_gameModel)) {
        CCLOG("GameController::initializeSubControllers - Failed to init UndoManager");
        return false;
    }

    // PlayFieldController::init(...)
    if (!_playfieldController->init(_gameModel, _undoManager)) {
        CCLOG("GameController::initializeSubControllers - Failed to init PlayFieldController");
        return false;
    }

    // StackController::init(...)
    if (!_stackController->init(_gameModel, _undoManager)) {
        CCLOG("GameController::initializeSubControllers - Failed to init StackController");
        return false;
    }

    // 设置回调
    _playfieldController->setCardClickCallback([this](bool success, std::shared_ptr<CardModel> cardModel) {
        onPlayFieldCardClicked(success, cardModel);
    });

    _stackController->setStackOperationCallback([this](bool success, std::shared_ptr<CardModel> cardModel) {
        onStackOperationPerformed(success, cardModel);
    });

    // 设置回退按钮回调
    if (_gameView) {
        _gameView->setUndoCallback([this]() {
            this->performUndo();
        });
    }

    // sub controllers initialized
    return true;
}

bool GameController::initializeSubControllerViews() {
    if (!_gameView || !_playfieldController || !_stackController) {
        CCLOG("GameController::initializeSubControllerViews - Missing components");
        return false;
    }

    // 按照README要求初始化各子控制器的视图：

    // PlayFieldController::initView(...)
    const auto& playfieldViews = _gameView->getPlayfieldCardViews();
    if (!_playfieldController->initView(playfieldViews)) {
        CCLOG("GameController::initializeSubControllerViews - Failed to init PlayFieldController view");
        return false;
    }

    // StackController::initView(...)
    const auto& stackViews = _gameView->getStackCardViews();
    auto currentCardView = _gameView->getCurrentCardView();
    if (!_stackController->initView(stackViews, currentCardView)) {
        CCLOG("GameController::initializeSubControllerViews - Failed to init StackController view");
        return false;
    }

    // 新增：将当前底牌视图传给 PlayFieldController
    _playfieldController->setCurrentCardView(_gameView->getCurrentCardView());
    
    // 新增：将当前底牌区域传给 PlayFieldController
    _playfieldController->setCurrentCardArea(_gameView->getCurrentCardArea());
    
    // 新增：设置GameView引用，用于同步更新
    _playfieldController->setGameView(_gameView);

    // sub controller views initialized
    return true;
}

bool GameController::initializeGameView() {
    if (!_gameView || !_levelConfig || !_gameModel) {
        return false;
    }
    
    return _gameView->initWithLevelConfig(_levelConfig, _gameModel);
}

void GameController::updateGameDisplay() {
    if (_gameView && _gameModel) {
        _gameView->updateDisplay(_gameModel);
    }
}

bool GameController::checkWinCondition() {
    if (!_gameModel) {
        return false;
    }
    
    return _gameModel->isGameWon();
}

void GameController::handleGameWin() {
    if (_gameModel) {
        _gameModel->setGameState(GameState::WIN);
        // player won
    }
}

void GameController::handleGameLose() {
    if (_gameModel) {
        _gameModel->setGameState(GameState::GAME_OVER);
        // game over
    }
}

void GameController::onPlayFieldCardClicked(bool success, std::shared_ptr<CardModel> cardModel) {
    if (success) {
        // playfield op ok

        // 检查胜利条件
        if (checkWinCondition()) {
            handleGameWin();
        }

        // 新增：同步 StackController 当前底牌视图，避免悬挂指针
        if (_stackController && _playfieldController) {
            _stackController->setCurrentCardView(_playfieldController->getCurrentCardView());
        }
    } else {
        CCLOG("GameController::onPlayFieldCardClicked - Playfield card operation failed");
    }
}

void GameController::onStackOperationPerformed(bool success, std::shared_ptr<CardModel> cardModel) {
    if (success) {
        // stack op ok

        // 关键修复：更新底牌显示
        this->updateCurrentCardDisplay();

        // 更新桌面牌的可匹配状态
        if (_playfieldController) {
            _playfieldController->updateDisplay();
        }
    } else {
        CCLOG("GameController::onStackOperationPerformed - Stack operation failed");
    }
}

bool GameController::performUndo() {
    // undo requested
    
    if (!_undoManager || !_gameModel) {
        CCLOG("GameController::performUndo - UndoManager or GameModel not available");
        return false;
    }
    
    if (!_undoManager->canUndo()) {
        CCLOG("GameController::performUndo - No undo operations available");
        return false;
    }
    
    // current undo count, performing
    
    // 执行撤销操作
    bool success = _undoManager->performUndo([this](bool undoSuccess, std::shared_ptr<UndoModel> undoModel) {
        if (undoSuccess && undoModel) {
            // undo ok
            
            // 执行回退动画
            this->performUndoAnimation(undoModel);
            
            // 更新显示
            this->updateGameDisplay();
        } else {
            CCLOG("GameController::performUndo - Undo failed");
        }
    });
    
    return success;
}

void GameController::performUndoAnimation(std::shared_ptr<UndoModel> undoModel) {
    if (!undoModel || !_gameView) {
        CCLOG("GameController::performUndoAnimation - Invalid parameters");
        return;
    }
    
    // start undo animation
    
    // 根据撤销操作类型执行相应的动画
    auto sourceCard = undoModel->getSourceCard();
    auto targetCard = undoModel->getTargetCard();
    
    if (!sourceCard) {
        CCLOG("GameController::performUndoAnimation - No source card in undo model");
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

void GameController::performPlayfieldCardUndoAnimation(std::shared_ptr<UndoModel> undoModel) {
    auto sourceCard = undoModel->getSourceCard();
    
    // 获取当前底牌视图（即将移动回桌面的卡牌）
    auto currentCardView = _gameView->getCurrentCardView();
    if (!currentCardView) {
        CCLOG("GameController::performPlayfieldCardUndoAnimation - No current card view found");
        return;
    }
    
    // === 关键调试信息 ===
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
    
    // 强制确保CardView显示正确的卡牌内容
    currentCardView->setCardModel(sourceCard);
    currentCardView->updateDisplay();
    currentCardView->setFlipped(true, false); // 强制设置为正面显示
    // forced update
    
    // 使用记录的世界坐标作为目标位置
    Vec2 worldTargetPos = undoModel->getSourcePosition(); // 这现在是正确的世界坐标
    
    // target pos
    
    // 使用与现有动画完全相同的坐标转换方式
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
    
    // 播放回退动画
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

void GameController::restoreCardToPlayfield(CardView* cardView, std::shared_ptr<CardModel> cardModel, const Vec2& absolutePos, int originalZOrder) {
    if (!cardView || !_playfieldController) {
        CCLOG("GameController::restoreCardToPlayfield - Invalid parameters");
        return;
    }
    
    // 获取桌面区域节点并计算正确的相对坐标
    auto playfieldArea = _gameView->getPlayfieldArea();
    if (!playfieldArea) {
        CCLOG("GameController::restoreCardToPlayfield - Playfield area not found");
        cardView->release();
        return;
    }
    
    // 将世界坐标转换为桌面区域的本地坐标（正确的坐标转换）
    Vec2 relativePos = playfieldArea->convertToNodeSpace(absolutePos);
    
    // restoring card to playfield
    
    // 更新卡牌模型位置
    cardModel->setPosition(relativePos);
    
    // 将卡牌重新添加到桌面区域
    cardView->retain();
    cardView->removeFromParent();
    // 使用与初始生成一致的层级（默认0），避免回退后层级过高
    playfieldArea->addChild(cardView);
    // 恢复原始本地z序
    cardView->setLocalZOrder(originalZOrder);
    // applied local z
    cardView->setPosition(relativePos);
    cardView->setEnabled(true); // 重新启用交互
    
    // 重新注册到GameView
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

void GameController::updateCurrentCardDisplay() {
    if (!_gameModel || !_gameView) {
        CCLOG("GameController::updateCurrentCardDisplay - Invalid game state");
        return;
    }
    
    // 获取当前底牌
    auto currentCard = _gameModel->getCurrentCard();
    if (!currentCard) {
        CCLOG("GameController::updateCurrentCardDisplay - No current card in model");
        // 底牌区域置空
        _gameView->setCurrentCardView(nullptr);
        return;
    }
    
    // updating current card
    
    // 创建新的底牌视图
    auto newCurrentCardView = CardView::create(currentCard);
    if (!newCurrentCardView) {
        CCLOG("GameController::updateCurrentCardDisplay - Failed to create card view");
        return;
    }
    
    // 获取底牌区域
    auto currentCardArea = _gameView->getCurrentCardArea();
    if (!currentCardArea) {
        CCLOG("GameController::updateCurrentCardDisplay - No current card area");
        return;
    }
    
    // 清除旧的底牌视图
    currentCardArea->removeAllChildren();
    
    // 添加新的底牌视图
    currentCardArea->addChild(newCurrentCardView, 300);
    newCurrentCardView->setPosition(Vec2(0, 0)); // 居中显示
    newCurrentCardView->setFlipped(true, false);  // 正面显示，无动画
    newCurrentCardView->setEnabled(false);        // 底牌不可点击
    
    // 更新GameView的引用
    _gameView->setCurrentCardView(newCurrentCardView);
    
    // 关键修复：同步更新PlayFieldController的引用
    if (_playfieldController) {
        _playfieldController->setCurrentCardView(newCurrentCardView);
    }
    
    // updated
}

void GameController::performStackCardUndoAnimation(std::shared_ptr<UndoModel> undoModel) {
    auto sourceCard = undoModel->getSourceCard();
    
    // creating view for card
    
    // 为要回退的卡牌创建新的视图（因为模型已经更新，当前底牌视图已经是恢复后的底牌）
    auto cardViewToAnimate = CardView::create(sourceCard);
    if (!cardViewToAnimate) {
        CCLOG("GameController::performStackCardUndoAnimation - Failed to create card view");
        return;
    }
    
    // 目标位置（手牌堆的绝对位置）
    Vec2 worldTargetPos = undoModel->getSourcePosition();
    
    // target pos
    
    // 获取底牌区域作为起始位置
    auto currentCardArea = _gameView->getCurrentCardArea();
    if (!currentCardArea) {
        CCLOG("GameController::performStackCardUndoAnimation - No current card area found");
        return;
    }
    
    // 使用与桌面牌回退相同的坐标转换方式
    Node* gameViewNode = currentCardArea->getParent();    // GameView（作为动画层）
    
    // 将新创建的卡牌视图放在底牌区域的中心作为动画起点
    Vec2 worldStart = currentCardArea->convertToWorldSpace(Vec2(0, 0)); // 底牌区域中心
    Vec2 worldTarget = worldTargetPos; // 使用记录的世界坐标
    
    Vec2 startInGameView = gameViewNode->convertToNodeSpace(worldStart);
    Vec2 targetInGameView = gameViewNode->convertToNodeSpace(worldTarget);
    
    // pos conversion
    
    // 设置动画卡牌的初始状态和位置
    cardViewToAnimate->setFlipped(true, false); // 正面显示
    cardViewToAnimate->setEnabled(false); // 动画期间不可点击
    
    // 将卡牌添加到动画层
    gameViewNode->addChild(cardViewToAnimate, 500); // 动画层
    cardViewToAnimate->setPosition(startInGameView);
    
    // 立即更新底牌显示（在动画开始前，避免视觉冲突）
    this->updateCurrentCardDisplay();
    // updated current card display
    
    // 播放回退动画
    _gameView->playCardMoveAnimation(cardViewToAnimate, targetInGameView, 0.5f, [this, cardViewToAnimate, worldTargetPos, sourceCard]() {
        // animation completed
        
        // 动画完成后，将卡牌重新添加到手牌堆
        this->restoreCardToStack(cardViewToAnimate, sourceCard, worldTargetPos);
    });
}

void GameController::restoreCardToStack(CardView* cardView, std::shared_ptr<CardModel> cardModel, const Vec2& absolutePos) {
    if (!cardView || !_stackController) {
        CCLOG("GameController::restoreCardToStack - Invalid parameters");
        return;
    }
    
    // 获取手牌堆区域节点并计算正确的相对坐标
    auto stackArea = _gameView->getStackArea();
    if (!stackArea) {
        CCLOG("GameController::restoreCardToStack - Stack area not found");
        cardView->release();
        return;
    }
    
    // 将世界坐标转换为手牌堆区域的本地坐标
    Vec2 relativePos = stackArea->convertToNodeSpace(absolutePos);
    
    // restoring card to stack
    
    // 更新卡牌模型位置
    cardModel->setPosition(relativePos);
    
    // 将卡牌重新添加到手牌堆区域
    cardView->retain();
    cardView->removeFromParent();
    stackArea->addChild(cardView, 100);
    cardView->setPosition(relativePos);
    cardView->setEnabled(true); // 重新启用交互（作为栈顶卡牌）
    
    // 重新注册到GameView
    auto& stackViews = const_cast<std::vector<CardView*>&>(_gameView->getStackCardViews());
    stackViews.push_back(cardView);
    
    auto& cardViewMap = const_cast<std::map<int, CardView*>&>(_gameView->getCardViewMap());
    cardViewMap[cardModel->getCardId()] = cardView;
    
    // 关键修复：注册到StackController
    if (_stackController) {
        _stackController->registerCardView(cardView);
    }
    
    // restored
    cardView->release();
}
