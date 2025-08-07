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
    CCLOG("GameController::init - Controller initialized successfully");

    return true;
}

bool GameController::startGame(int levelId) {
    if (!_isInitialized) {
        CCLOG("GameController::startGame - Controller not initialized");
        return false;
    }

    CCLOG("GameController::startGame - Starting level %d", levelId);

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

    CCLOG("GameController::startGame - Game started successfully: %s",
          _levelConfig->getSummary().c_str());

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
        CCLOG("GameController::pauseGame - Game paused");
    }
}

void GameController::resumeGame() {
    if (_gameModel) {
        _gameModel->setGameState(GameState::PLAYING);
        CCLOG("GameController::resumeGame - Game resumed");
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

    CCLOG("GameController::initializeSubControllers - All sub controllers initialized");
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

    CCLOG("GameController::initializeSubControllerViews - All sub controller views initialized");
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
        CCLOG("GameController::handleGameWin - Player won the game!");
    }
}

void GameController::handleGameLose() {
    if (_gameModel) {
        _gameModel->setGameState(GameState::GAME_OVER);
        CCLOG("GameController::handleGameLose - Game over!");
    }
}

void GameController::onPlayFieldCardClicked(bool success, std::shared_ptr<CardModel> cardModel) {
    if (success) {
        CCLOG("GameController::onPlayFieldCardClicked - Playfield card operation successful: %s",
              cardModel ? cardModel->toString().c_str() : "Unknown");

        // 检查胜利条件
        if (checkWinCondition()) {
            handleGameWin();
        }
    } else {
        CCLOG("GameController::onPlayFieldCardClicked - Playfield card operation failed");
    }
}

void GameController::onStackOperationPerformed(bool success, std::shared_ptr<CardModel> cardModel) {
    if (success) {
        CCLOG("GameController::onStackOperationPerformed - Stack operation successful: %s",
              cardModel ? cardModel->toString().c_str() : "Unknown");

        // 更新桌面牌的可匹配状态
        if (_playfieldController) {
            _playfieldController->updateDisplay();
        }
    } else {
        CCLOG("GameController::onStackOperationPerformed - Stack operation failed");
    }
}
