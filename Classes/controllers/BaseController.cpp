#include "BaseController.h"

BaseController::BaseController() 
    : _gameModel(nullptr)
    , _undoManager(nullptr)
    , _configManager(nullptr) {
}

BaseController::~BaseController() {
    // 基类析构，派生类负责清理各自的资源
}

bool BaseController::initBase(std::shared_ptr<GameModel> gameModel, UndoManager* undoManager) {
    if (!gameModel || !undoManager) {
        CCLOG("BaseController::initBase - Invalid parameters: gameModel=%p, undoManager=%p", 
              gameModel.get(), undoManager);
        return false;
    }
    
    _gameModel = gameModel;
    _undoManager = undoManager;
    _configManager = ConfigManager::getInstance();
    
    if (!_configManager) {
        CCLOG("BaseController::initBase - Failed to get ConfigManager instance");
        return false;
    }
    
    return true;
}

void BaseController::playMoveAnimation(CardView* cardView, const Vec2& targetPosition,
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

bool BaseController::recordUndoOperationBase(std::shared_ptr<CardModel> sourceCard,
                                            std::shared_ptr<CardModel> targetCard,
                                            const Vec2& sourcePosition,
                                            const Vec2& targetPosition,
                                            int sourceStackIndex,
                                            int sourceZOrder,
                                            UndoOperationType operationType) {
    if (!_undoManager || !sourceCard || !targetCard) {
        return false;
    }
    
    // 根据操作类型创建相应的撤销记录
    std::shared_ptr<UndoModel> undoModel;
    
    switch (operationType) {
        case UndoOperationType::CARD_MOVE:
            undoModel = UndoModel::createPlayfieldToCurrentAction(
                sourceCard, targetCard,
                sourcePosition, targetPosition,
                0, sourceZOrder
            );
            break;
            
        case UndoOperationType::STACK_OPERATION:
            undoModel = UndoModel::createStackToCurrentAction(
                sourceCard, targetCard,
                sourcePosition, targetPosition,
                0
            );
            break;
            
        default:
            CCLOG("BaseController::recordUndoOperationBase - Unknown operation type: %d", 
                  static_cast<int>(operationType));
            return false;
    }
    
    if (!undoModel) {
        CCLOG("BaseController::recordUndoOperationBase - Failed to create undo model");
        return false;
    }
    
    return _undoManager->recordUndo(undoModel);
}

Vec2 BaseController::getWorldPosition(CardView* cardView) {
    if (!cardView || !cardView->getParent()) {
        return Vec2::ZERO;
    }
    
    Node* parent = cardView->getParent();
    return parent->convertToWorldSpace(cardView->getPosition());
}

Node* BaseController::getOverlayParent(CardView* cardView) {
    if (!cardView || !cardView->getParent()) {
        return nullptr;
    }
    
    Node* parent = cardView->getParent();
    // 通常覆盖层是父节点的父节点（GameView）
    return (parent && parent->getParent()) ? parent->getParent() : parent;
}

BaseController::AnimationCoordinates BaseController::calculateAnimationCoordinates(
    CardView* sourceCardView, 
    const Vec2& targetWorldPosition,
    Node* overlayParent) {
    
    AnimationCoordinates coords;
    coords.startPosition = Vec2::ZERO;
    coords.targetPosition = Vec2::ZERO;
    
    if (!sourceCardView) {
        return coords;
    }
    
    // 如果没有指定覆盖层父节点，自动获取
    if (!overlayParent) {
        overlayParent = getOverlayParent(sourceCardView);
    }
    
    if (!overlayParent) {
        return coords;
    }
    
    // 获取源位置的世界坐标
    Vec2 sourceWorldPosition = getWorldPosition(sourceCardView);
    
    // 转换为覆盖层坐标系
    coords.startPosition = overlayParent->convertToNodeSpace(sourceWorldPosition);
    coords.targetPosition = overlayParent->convertToNodeSpace(targetWorldPosition);
    
    return coords;
}

void BaseController::moveCardWithAnimation(CardView* cardView, 
                                          const Vec2& targetWorldPosition,
                                          int animationZOrder,
                                          const AnimationCallback& callback) {
    if (!cardView) {
        if (callback) callback(false);
        return;
    }
    
    // 获取覆盖层父节点
    Node* overlayParent = getOverlayParent(cardView);
    if (!overlayParent) {
        CCLOG("BaseController::moveCardWithAnimation - No overlay parent found");
        if (callback) callback(false);
        return;
    }
    
    // 计算动画坐标
    auto coords = calculateAnimationCoordinates(cardView, targetWorldPosition, overlayParent);
    
    // 提升到覆盖层并设置起始位置
    cardView->retain();
    cardView->removeFromParent();
    overlayParent->addChild(cardView, animationZOrder);
    cardView->setPosition(coords.startPosition);
    
    // 播放移动动画
    auto animationConfig = _configManager->getAnimationConfig();
    cardView->playMoveAnimation(coords.targetPosition, 
                               animationConfig->getMoveAnimationDuration(), 
                               [callback]() {
        if (callback) callback(true);
    });
}

template<typename T>
bool BaseController::isValidPointer(T* ptr, const std::string& errorMsg) {
    if (!ptr) {
        if (!errorMsg.empty()) {
            CCLOG("BaseController::isValidPointer - %s", errorMsg.c_str());
        }
        return false;
    }
    return true;
}
