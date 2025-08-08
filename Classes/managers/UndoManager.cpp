#include "UndoManager.h"

UndoManager::UndoManager()
    : _gameModel(nullptr)
    , _configManager(nullptr)
    , _maxUndoSteps(10)  // 默认值，将从配置中读取
    , _isInitialized(false) {
}

UndoManager::~UndoManager() {
    clearUndoHistory();
}

bool UndoManager::init(std::shared_ptr<GameModel> gameModel) {
    if (!gameModel) {
        CCLOG("UndoManager::init - Invalid game model");
        return false;
    }

    _gameModel = gameModel;

    // 获取配置管理器并读取撤销设置
    _configManager = ConfigManager::getInstance();
    if (_configManager) {
        auto gameRulesConfig = _configManager->getGameRulesConfig();
        if (gameRulesConfig->isUndoEnabled()) {
            _maxUndoSteps = gameRulesConfig->getMaxUndoSteps();
        } else {
            _maxUndoSteps = 0; // 禁用撤销
        }
    }

    _isInitialized = true;

    CCLOG("UndoManager::init - Initialized with max undo steps: %d", _maxUndoSteps);
    return true;
}

bool UndoManager::recordUndo(std::shared_ptr<UndoModel> undoModel) {
    if (!_isInitialized || !undoModel) {
        CCLOG("UndoManager::recordUndo - Manager not initialized or invalid undo model");
        return false;
    }
    
    if (!validateUndoOperation(undoModel)) {
        CCLOG("UndoManager::recordUndo - Invalid undo operation");
        return false;
    }
    
    // 添加到撤销栈
    _undoStack.push_back(undoModel);
    
    // 清理超出限制的记录
    cleanupExcessUndoRecords();
    
    CCLOG("UndoManager::recordUndo - Recorded undo operation: %s (Total: %d)", 
          undoModel->getOperationSummary().c_str(), static_cast<int>(_undoStack.size()));
    
    return true;
}

bool UndoManager::performUndo(const UndoCallback& callback) {
    if (!canUndo()) {
        CCLOG("UndoManager::performUndo - No undo operations available");
        if (callback) {
            callback(false, nullptr);
        }
        return false;
    }
    
    // 获取最后一个撤销操作
    auto undoModel = _undoStack.back();
    _undoStack.pop_back();
    
    CCLOG("UndoManager::performUndo - Performing undo: %s", 
          undoModel->getOperationSummary().c_str());
    
    // 打印详细的撤销信息
    auto sourceCard = undoModel->getSourceCard();
    auto targetCard = undoModel->getTargetCard();
    if (sourceCard) {
        CCLOG("UndoManager::performUndo - Source card: %s, position: (%.0f, %.0f)",
              sourceCard->toString().c_str(), 
              undoModel->getSourcePosition().x, undoModel->getSourcePosition().y);
    }
    if (targetCard) {
        CCLOG("UndoManager::performUndo - Target card: %s, position: (%.0f, %.0f)",
              targetCard->toString().c_str(),
              undoModel->getTargetPosition().x, undoModel->getTargetPosition().y);
    }
    
    // 应用撤销操作
    bool success = applyUndoToGameModel(undoModel);
    
    if (callback) {
        callback(success, undoModel);
    }
    
    CCLOG("UndoManager::performUndo - Undo %s (Remaining: %d)", 
          success ? "successful" : "failed", static_cast<int>(_undoStack.size()));
    
    return success;
}

bool UndoManager::canUndo() const {
    return _isInitialized && !_undoStack.empty();
}

int UndoManager::getUndoCount() const {
    return static_cast<int>(_undoStack.size());
}

void UndoManager::clearUndoHistory() {
    _undoStack.clear();
    CCLOG("UndoManager::clearUndoHistory - Cleared all undo history");
}

void UndoManager::setMaxUndoSteps(int maxSteps) {
    if (maxSteps <= 0) {
        CCLOG("UndoManager::setMaxUndoSteps - Invalid max steps: %d", maxSteps);
        return;
    }
    
    _maxUndoSteps = maxSteps;
    cleanupExcessUndoRecords();
    
    CCLOG("UndoManager::setMaxUndoSteps - Set max undo steps to: %d", _maxUndoSteps);
}

std::shared_ptr<UndoModel> UndoManager::getLastUndoOperation() const {
    if (_undoStack.empty()) {
        return nullptr;
    }
    return _undoStack.back();
}

std::vector<std::string> UndoManager::getUndoSummary() const {
    std::vector<std::string> summary;
    
    for (size_t i = 0; i < _undoStack.size(); i++) {
        const auto& undoModel = _undoStack[i];
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%zu. %s", 
                i + 1, undoModel->getOperationSummary().c_str());
        summary.push_back(std::string(buffer));
    }
    
    return summary;
}

bool UndoManager::applyUndoToGameModel(std::shared_ptr<UndoModel> undoModel) {
    if (!_gameModel || !undoModel) {
        return false;
    }
    
    // 根据撤销操作类型执行相应的恢复逻辑
    switch (undoModel->getOperationType()) {
        case UndoOperationType::CARD_MOVE:
            return _gameModel->undoCardMove(undoModel);
            
        case UndoOperationType::CARD_FLIP:
            return _gameModel->undoCardFlip(undoModel);
            
        case UndoOperationType::STACK_OPERATION:
            return _gameModel->undoStackOperation(undoModel);
            
        default:
            CCLOG("UndoManager::applyUndoToGameModel - Unknown operation type: %d", 
                  static_cast<int>(undoModel->getOperationType()));
            return false;
    }
}

bool UndoManager::validateUndoOperation(std::shared_ptr<UndoModel> undoModel) const {
    if (!undoModel) {
        return false;
    }
    
    // 检查操作类型是否有效
    UndoOperationType opType = undoModel->getOperationType();
    if (opType == UndoOperationType::NONE) {
        return false;
    }
    
    // 检查是否有必要的数据
    if (undoModel->getOperationSummary().empty()) {
        return false;
    }
    
    return true;
}

void UndoManager::cleanupExcessUndoRecords() {
    while (static_cast<int>(_undoStack.size()) > _maxUndoSteps) {
        _undoStack.erase(_undoStack.begin());
    }
}
