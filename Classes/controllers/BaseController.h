#ifndef __BASE_CONTROLLER_H__
#define __BASE_CONTROLLER_H__

#include "cocos2d.h"
#include "../models/GameModel.h"
#include "../models/CardModel.h"
#include "../models/UndoModel.h"
#include "../views/CardView.h"
#include "../managers/UndoManager.h"
#include "../managers/ConfigManager.h"
#include <memory>
#include <vector>
#include <functional>

USING_NS_CC;

/**
 * 基础控制器类
 * 提供所有控制器的共同功能，避免代码重复
 */
class BaseController {
public:
    /**
     * 动画完成回调
     * @param success 动画是否成功完成
     */
    using AnimationCallback = std::function<void(bool success)>;
    
    /**
     * 构造函数
     */
    BaseController();
    
    /**
     * 虚析构函数
     */
    virtual ~BaseController();
    
    /**
     * 基础初始化方法
     * @param gameModel 游戏数据模型
     * @param undoManager 撤销管理器
     * @return 是否初始化成功
     */
    virtual bool initBase(std::shared_ptr<GameModel> gameModel, UndoManager* undoManager);
    
protected:
    /**
     * 播放移动动画的通用方法
     * @param cardView 要移动的卡牌视图
     * @param targetPosition 目标位置
     * @param callback 动画完成回调
     */
    void playMoveAnimation(CardView* cardView, const Vec2& targetPosition,
                          const AnimationCallback& callback);
    
    /**
     * 记录撤销操作的通用方法
     * @param sourceCard 源卡牌
     * @param targetCard 目标卡牌
     * @param sourcePosition 源位置
     * @param targetPosition 目标位置
     * @param sourceStackIndex 源堆栈索引
     * @param sourceZOrder 源Z顺序
     * @param operationType 操作类型
     * @return 是否记录成功
     */
    bool recordUndoOperationBase(std::shared_ptr<CardModel> sourceCard,
                                std::shared_ptr<CardModel> targetCard,
                                const Vec2& sourcePosition,
                                const Vec2& targetPosition,
                                int sourceStackIndex,
                                int sourceZOrder,
                                UndoOperationType operationType);
    
    /**
     * 获取世界坐标的通用方法
     * @param cardView 卡牌视图
     * @return 世界坐标
     */
    Vec2 getWorldPosition(CardView* cardView);
    
    /**
     * 获取覆盖层父节点（通常是GameView）
     * @param cardView 卡牌视图
     * @return 覆盖层父节点
     */
    Node* getOverlayParent(CardView* cardView);
    
    /**
     * 计算动画坐标对（世界坐标转换为覆盖层坐标）
     * @param sourceCardView 源卡牌视图
     * @param targetWorldPosition 目标世界坐标
     * @param overlayParent 覆盖层父节点
     * @return 包含起始和目标位置的坐标对
     */
    struct AnimationCoordinates {
        Vec2 startPosition;
        Vec2 targetPosition;
    };
    AnimationCoordinates calculateAnimationCoordinates(CardView* sourceCardView, 
                                                      const Vec2& targetWorldPosition,
                                                      Node* overlayParent = nullptr);
    
    /**
     * 通用的卡牌动画移动方法
     * @param cardView 要移动的卡牌视图
     * @param targetWorldPosition 目标世界坐标
     * @param animationZOrder 动画时的Z顺序
     * @param callback 动画完成回调
     */
    void moveCardWithAnimation(CardView* cardView, 
                              const Vec2& targetWorldPosition,
                              int animationZOrder,
                              const AnimationCallback& callback);
    
    /**
     * 安全的空指针检查
     * @param ptr 要检查的指针
     * @param errorMsg 错误消息
     * @return 是否为有效指针
     */
    template<typename T>
    bool isValidPointer(T* ptr, const std::string& errorMsg = "");
    
protected:
    // 共同的成员变量
    std::shared_ptr<GameModel> _gameModel;
    UndoManager* _undoManager;
    ConfigManager* _configManager;
};

#endif // __BASE_CONTROLLER_H__
