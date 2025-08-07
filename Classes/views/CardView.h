#ifndef __CARD_VIEW_H__
#define __CARD_VIEW_H__

#include "cocos2d.h"
#include "../models/CardModel.h"
#include "../configs/loaders/ConfigManager.h"
#include <functional>
#include <memory>

USING_NS_CC;

/**
 * 卡牌视图组件
 * 负责卡牌的显示、动画和触摸事件处理
 */
class CardView : public Node {
public:
    /**
     * 创建卡牌视图
     * @param cardModel 卡牌数据模型
     * @return 卡牌视图实例
     */
    static CardView* create(std::shared_ptr<CardModel> cardModel);
    
    /**
     * 初始化卡牌视图
     * @param cardModel 卡牌数据模型
     * @return 是否初始化成功
     */
    bool initWithCardModel(std::shared_ptr<CardModel> cardModel);
    
    /**
     * 析构函数
     */
    virtual ~CardView();
    
    // 卡牌模型相关
    std::shared_ptr<CardModel> getCardModel() const { return _cardModel; }
    void setCardModel(std::shared_ptr<CardModel> cardModel);
    
    // 触摸事件回调
    using CardClickCallback = std::function<void(CardView*, std::shared_ptr<CardModel>)>;
    void setCardClickCallback(const CardClickCallback& callback) { _cardClickCallback = callback; }
    
    // 视觉状态
    void setFlipped(bool flipped, bool animated = true);
    bool isFlipped() const;
    
    void setHighlighted(bool highlighted);
    bool isHighlighted() const { return _isHighlighted; }
    
    void setEnabled(bool enabled);
    bool isEnabled() const { return _isEnabled; }
    
    // 动画方法
    /**
     * 播放移动动画
     * @param targetPosition 目标位置
     * @param duration 动画时长
     * @param callback 完成回调
     */
    void playMoveAnimation(const Vec2& targetPosition, float duration = 0.3f, 
                          const std::function<void()>& callback = nullptr);
    
    /**
     * 播放翻牌动画
     * @param flipped 目标翻牌状态
     * @param duration 动画时长
     * @param callback 完成回调
     */
    void playFlipAnimation(bool flipped, float duration = 0.2f,
                          const std::function<void()>& callback = nullptr);
    
    /**
     * 播放高亮动画
     * @param highlighted 是否高亮
     */
    void playHighlightAnimation(bool highlighted);
    
    /**
     * 播放缩放动画
     * @param scale 目标缩放
     * @param duration 动画时长
     */
    void playScaleAnimation(float scale, float duration = 0.1f);
    
    // 卡牌尺寸 - 动态获取实际图片尺寸
    Size getCardSize() const {
        // 返回背景图片的实际尺寸
        if (_cardBackground) {
            return _cardBackground->getContentSize();
        }
        return Size(100, 140); // 默认尺寸，仅在图片加载失败时使用
    }
    
    // 更新显示
    void updateDisplay();

    /**
     * 更新卡牌布局（根据实际图片尺寸）
     */
    void updateCardLayout();

protected:
    /**
     * 构造函数
     */
    CardView();
    
    /**
     * 初始化触摸事件
     */
    void initTouchEvents();
    
    /**
     * 创建卡牌背景
     */
    void createCardBackground();
    
    /**
     * 创建卡牌正面
     */
    void createCardFront();
    
    /**
     * 创建卡牌背面
     */
    void createCardBack();
    
    /**
     * 更新卡牌正面显示
     */
    void updateCardFront();
    
    /**
     * 触摸开始事件
     */
    bool onTouchBegan(Touch* touch, Event* event);
    
    /**
     * 触摸结束事件
     */
    void onTouchEnded(Touch* touch, Event* event);
    
    /**
     * 触摸取消事件
     */
    void onTouchCancelled(Touch* touch, Event* event);

private:
    std::shared_ptr<CardModel> _cardModel;      // 卡牌数据模型
    CardClickCallback _cardClickCallback;       // 点击回调
    ConfigManager* _configManager;              // 配置管理器
    
    // UI组件
    Sprite* _cardBackground;                    // 卡牌背景
    Node* _cardFront;                          // 卡牌正面
    Node* _cardBack;                           // 卡牌背面
    Sprite* _bigNumberSprite;                  // 大数字精灵（中间）
    Sprite* _smallNumberSprite;                // 小数字精灵（左上角）
    Sprite* _suitSprite;                       // 花色精灵（右上角）
    
    // 状态
    bool _isHighlighted;                       // 是否高亮
    bool _isEnabled;                           // 是否可用
    bool _isAnimating;                         // 是否正在动画
    
    // 触摸相关
    EventListenerTouchOneByOne* _touchListener; // 触摸监听器
    
    /**
     * 获取花色颜色
     * @param suit 花色类型
     * @return 颜色
     */
    Color3B getSuitColor(CardSuitType suit) const;
    
    /**
     * 创建卡牌边框
     * @return 边框精灵
     */
    Sprite* createCardBorder() const;

    /**
     * 获取牌面文字
     * @param face 牌面类型
     * @return 牌面文字
     */
    std::string getFaceText(CardFaceType face) const;

    /**
     * 获取花色图片路径
     * @param suit 花色类型
     * @return 图片路径
     */
    std::string getSuitImagePath(CardSuitType suit) const;
};

#endif // __CARD_VIEW_H__
