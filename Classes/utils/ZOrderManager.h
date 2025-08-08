#ifndef __Z_ORDER_MANAGER_H__
#define __Z_ORDER_MANAGER_H__

/**
 * Z-Order 管理器
 * 统一管理所有UI元素的层级，避免z-order冲突
 */
class ZOrderManager {
public:
    // 基础层级定义
    enum class Layer : int {
        BACKGROUND = 0,          // 背景层
        GAME_BOARD = 100,        // 游戏板面
        CARDS_NORMAL = 200,      // 普通卡牌层
        CARDS_CURRENT = 300,     // 当前底牌层
        ANIMATION = 500,         // 动画层（动画过程中的卡牌）
        UI_OVERLAY = 800,        // UI覆盖层
        DEBUG = 900              // 调试层
    };
    
    /**
     * 获取指定层级的基础z-order值
     */
    static int getLayerZOrder(Layer layer) {
        return static_cast<int>(layer);
    }
    
    /**
     * 在指定层级内获取相对z-order
     * @param layer 目标层级
     * @param offset 在该层级内的偏移量（0-99）
     */
    static int getZOrder(Layer layer, int offset = 0) {
        return static_cast<int>(layer) + offset;
    }
    
    /**
     * 获取动画层的z-order（保证在最前面）
     */
    static int getAnimationZOrder() {
        return getZOrder(Layer::ANIMATION, 50);
    }
    
    /**
     * 获取当前底牌的z-order
     */
    static int getCurrentCardZOrder() {
        return getZOrder(Layer::CARDS_CURRENT, 10);
    }
    
    /**
     * 获取普通卡牌的z-order
     */
    static int getNormalCardZOrder(int cardIndex = 0) {
        return getZOrder(Layer::CARDS_NORMAL, cardIndex % 50); // 最多50张卡牌重叠
    }
};

#endif // __Z_ORDER_MANAGER_H__
