#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include "cocos2d.h"
#include "../models/UndoModel.h"
#include "../models/GameModel.h"
#include "../configs/loaders/ConfigManager.h"
#include <memory>
#include <vector>
#include <functional>

USING_NS_CC;

/**
 * 撤销管理器
 * 负责管理游戏中的撤销操作，记录和恢复游戏状态
 * 作为Controller的成员变量，可持有Model数据并对其进行加工
 * 禁止实现为单例模式，禁止反向依赖Controller
 */
class UndoManager {
public:
    /**
     * 撤销操作完成回调
     * @param success 是否成功
     * @param undoModel 撤销的操作数据
     */
    using UndoCallback = std::function<void(bool success, std::shared_ptr<UndoModel> undoModel)>;
    
    /**
     * 构造函数
     */
    UndoManager();
    
    /**
     * 析构函数
     */
    virtual ~UndoManager();
    
    /**
     * 初始化撤销管理器
     * @param gameModel 游戏数据模型
     * @return 是否初始化成功
     */
    bool init(std::shared_ptr<GameModel> gameModel);
    
    /**
     * 记录一个撤销操作
     * @param undoModel 撤销操作数据
     * @return 是否记录成功
     */
    bool recordUndo(std::shared_ptr<UndoModel> undoModel);
    
    /**
     * 执行撤销操作
     * @param callback 完成回调
     * @return 是否可以撤销
     */
    bool performUndo(const UndoCallback& callback = nullptr);
    
    /**
     * 检查是否可以撤销
     * @return 是否可以撤销
     */
    bool canUndo() const;
    
    /**
     * 获取可撤销的操作数量
     * @return 操作数量
     */
    int getUndoCount() const;
    
    /**
     * 清除所有撤销记录
     */
    void clearUndoHistory();
    
    /**
     * 设置最大撤销步数
     * @param maxSteps 最大步数
     */
    void setMaxUndoSteps(int maxSteps);
    
    /**
     * 获取最大撤销步数
     * @return 最大步数
     */
    int getMaxUndoSteps() const { return _maxUndoSteps; }
    
    /**
     * 获取最近的撤销操作（不执行）
     * @return 最近的撤销操作，无操作时返回nullptr
     */
    std::shared_ptr<UndoModel> getLastUndoOperation() const;
    
    /**
     * 获取所有撤销操作的摘要信息
     * @return 摘要信息列表
     */
    std::vector<std::string> getUndoSummary() const;

protected:
    /**
     * 应用撤销操作到游戏模型
     * @param undoModel 撤销操作数据
     * @return 是否应用成功
     */
    bool applyUndoToGameModel(std::shared_ptr<UndoModel> undoModel);
    
    /**
     * 验证撤销操作的有效性
     * @param undoModel 撤销操作数据
     * @return 是否有效
     */
    bool validateUndoOperation(std::shared_ptr<UndoModel> undoModel) const;
    
    /**
     * 清理超出限制的撤销记录
     */
    void cleanupExcessUndoRecords();

private:
    std::shared_ptr<GameModel> _gameModel;              // 游戏数据模型
    std::vector<std::shared_ptr<UndoModel>> _undoStack; // 撤销操作栈
    ConfigManager* _configManager;                      // 配置管理器
    int _maxUndoSteps;                                  // 最大撤销步数
    bool _isInitialized;                                // 是否已初始化
};

#endif // __UNDO_MANAGER_H__
