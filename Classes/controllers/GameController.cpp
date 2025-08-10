#include "GameController.h"

GameController::GameController()
    : _gameView(nullptr)
    , _gameModel(nullptr)
    , _levelConfig(nullptr)
    , _playfieldController(nullptr)
    , _stackController(nullptr)
    , _undoManager(nullptr)
    , _undoController(nullptr)
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

    if (_undoController) {
        delete _undoController;
        _undoController = nullptr;
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
    _undoController = new UndoController();

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
    if (!_gameModel || !_playfieldController || !_stackController || !_undoManager || !_undoController) {
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

    // 初始化UndoController
    if (!_undoController->init(_gameView, _gameModel, _undoManager, _playfieldController, _stackController)) {
        CCLOG("GameController::initializeSubControllers - Failed to init UndoController");
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
    // 委托给UndoController处理
    if (!_undoController) {
        CCLOG("GameController::performUndo - UndoController not available");
        return false;
    }
    
    return _undoController->performUndo();
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
