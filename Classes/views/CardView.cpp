#include "CardView.h"

// 移除固定尺寸，改为使用实际图片尺寸

CardView* CardView::create(std::shared_ptr<CardModel> cardModel) {
    CardView* cardView = new (std::nothrow) CardView();
    if (cardView && cardView->initWithCardModel(cardModel)) {
        cardView->autorelease();
        return cardView;
    }
    CC_SAFE_DELETE(cardView);
    return nullptr;
}

CardView::CardView()
    : _cardModel(nullptr)
    , _configManager(nullptr)
    , _cardBackground(nullptr)
    , _cardFront(nullptr)
    , _cardBack(nullptr)
    , _bigNumberSprite(nullptr)
    , _smallNumberSprite(nullptr)
    , _suitSprite(nullptr)
    , _isHighlighted(false)
    , _isEnabled(true)
    , _isAnimating(false)
    , _touchListener(nullptr) {
}

CardView::~CardView() {
    if (_touchListener) {
        _eventDispatcher->removeEventListener(_touchListener);
        _touchListener = nullptr;
    }
}

bool CardView::initWithCardModel(std::shared_ptr<CardModel> cardModel) {
    if (!Node::init()) {
        return false;
    }

    _cardModel = cardModel;

    // 设置卡牌节点的锚点为中心，使位置坐标表示卡牌中心
    setAnchorPoint(Vec2(0.5f, 0.5f));

    // 获取配置管理器
    _configManager = ConfigManager::getInstance();
    if (!_configManager) {
        CCLOG("CardView::initWithCardModel - Failed to get ConfigManager");
        return false;
    }

    // 创建卡牌组件（背景图片会决定实际尺寸）
    createCardBackground();
    createCardFront();
    createCardBack();

    // 初始化触摸事件
    initTouchEvents();

    // 更新显示
    updateDisplay();

    return true;
}

void CardView::setCardModel(std::shared_ptr<CardModel> cardModel) {
    _cardModel = cardModel;
    updateDisplay();
}

void CardView::setFlipped(bool flipped, bool animated) {
    if (!_cardModel) return;
    
    if (_cardModel->isFlipped() == flipped) return;
    
    _cardModel->setFlipped(flipped);
    
    if (animated) {
        playFlipAnimation(flipped);
    } else {
        updateDisplay();
    }
}

bool CardView::isFlipped() const {
    return _cardModel ? _cardModel->isFlipped() : false;
}

void CardView::setHighlighted(bool highlighted) {
    if (_isHighlighted == highlighted) return;
    
    _isHighlighted = highlighted;
    playHighlightAnimation(highlighted);
}

void CardView::setEnabled(bool enabled) {
    _isEnabled = enabled;
    // 不再修改颜色，仅影响交互（onTouchBegan 中会判断）
}

void CardView::setDimmed(bool dimmed) {
    if (_cardBackground) {
        _cardBackground->setColor(dimmed ? Color3B(128,128,128) : Color3B::WHITE);
    }
}

void CardView::playMoveAnimation(const Vec2& targetPosition, float duration, 
                                const std::function<void()>& callback) {
    if (_isAnimating) return;
    
    _isAnimating = true;
    
    auto moveAction = MoveTo::create(duration, targetPosition);
    auto callbackAction = CallFunc::create([this, callback]() {
        _isAnimating = false;
        if (callback) {
            callback();
        }
    });
    
    auto sequence = Sequence::create(moveAction, callbackAction, nullptr);
    runAction(sequence);
}

void CardView::playFlipAnimation(bool flipped, float duration, 
                                const std::function<void()>& callback) {
    if (_isAnimating) return;
    
    _isAnimating = true;
    
    // 翻牌动画：先缩放到0，切换显示，再缩放回来
    auto scaleDown = ScaleTo::create(duration * 0.5f, 0.0f, 1.0f);
    auto switchDisplay = CallFunc::create([this, flipped]() {
        updateDisplay();
    });
    auto scaleUp = ScaleTo::create(duration * 0.5f, 1.0f, 1.0f);
    auto callbackAction = CallFunc::create([this, callback]() {
        _isAnimating = false;
        if (callback) {
            callback();
        }
    });
    
    auto sequence = Sequence::create(scaleDown, switchDisplay, scaleUp, callbackAction, nullptr);
    runAction(sequence);
}

void CardView::playHighlightAnimation(bool highlighted) {
    if (highlighted) {
        // 高亮效果：轻微放大和发光
        auto scaleUp = ScaleTo::create(0.1f, 1.1f);
        auto scaleDown = ScaleTo::create(0.1f, 1.0f);
        auto sequence = Sequence::create(scaleUp, scaleDown, nullptr);
        runAction(sequence);

        // 改变颜色为高亮色
        if (_cardBackground) {
            _cardBackground->setColor(Color3B(255, 255, 150));
        }
    } else {
        // 恢复正常颜色
        if (_cardBackground) {
            _cardBackground->setColor(Color3B::WHITE);
        }
    }
}

void CardView::playScaleAnimation(float scale, float duration) {
    auto scaleAction = ScaleTo::create(duration, scale);
    runAction(scaleAction);
}

void CardView::updateDisplay() {
    if (!_cardModel) return;

    bool flipped = _cardModel->isFlipped();

    // 显示/隐藏正面和背面
    if (_cardFront) {
        _cardFront->setVisible(flipped);
    }
    if (_cardBack) {
        _cardBack->setVisible(!flipped);
    }

    // 更新正面显示
    if (flipped) {
        updateCardFront();
    }

    // 暂时不更新位置，避免位置重置问题
    // setPosition(_cardModel->getPosition());
}

void CardView::updateCardLayout() {
    if (!_cardBackground) return;

    Size actualSize = _cardBackground->getContentSize();
    CCLOG("Updating layout with actual size: %.0f x %.0f", actualSize.width, actualSize.height);

    // 更新节点内容尺寸
    if (_cardFront) {
        _cardFront->setContentSize(actualSize);
    }
    if (_cardBack) {
        _cardBack->setContentSize(actualSize);
    }

    // 使用配置中的卡牌布局
    auto cardLayoutConfig = _configManager->getCardLayoutConfig();

    // 根据实际卡牌尺寸精确定位元素
    if (_bigNumberSprite) {
        // 大数字位置使用配置
        Vec2 bigPos = cardLayoutConfig->getBigNumberAbsolutePosition(actualSize);
        _bigNumberSprite->setPosition(bigPos);
        CCLOG("Big number position: %.0f, %.0f", bigPos.x, bigPos.y);
    }

    if (_smallNumberSprite) {
        // 小数字位置使用配置
        Vec2 smallPos = cardLayoutConfig->getSmallNumberAbsolutePosition(actualSize);
        _smallNumberSprite->setPosition(smallPos);
        CCLOG("Small number position: %.0f, %.0f", smallPos.x, smallPos.y);
    }

    if (_suitSprite) {
        // 花色位置使用配置
        Vec2 suitPos = cardLayoutConfig->getSuitAbsolutePosition(actualSize);
        _suitSprite->setPosition(suitPos);
        CCLOG("Suit position: %.0f, %.0f", suitPos.x, suitPos.y);
    }

    // 更新背面标签位置
    if (_cardBack) {
        auto backLabel = _cardBack->getChildByName("back_label");
        if (backLabel) {
            Vec2 backPos = cardLayoutConfig->getCardBackTextAbsolutePosition(actualSize);
            backLabel->setPosition(backPos);
        }
    }
}

void CardView::initTouchEvents() {
    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);

    _touchListener->onTouchBegan = CC_CALLBACK_2(CardView::onTouchBegan, this);
    _touchListener->onTouchEnded = CC_CALLBACK_2(CardView::onTouchEnded, this);
    _touchListener->onTouchCancelled = CC_CALLBACK_2(CardView::onTouchCancelled, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
}

void CardView::createCardBackground() {
    // 使用实际的卡牌底图，保持原始尺寸
    _cardBackground = Sprite::create("res/card_general.png");
    if (_cardBackground) {
        // 不修改图片尺寸，保持原始大小
        _cardBackground->setAnchorPoint(Vec2(0.5f, 0.5f));

        // 根据背景图片的实际尺寸设置节点尺寸和位置
        Size actualSize = _cardBackground->getContentSize();
        setContentSize(actualSize);
        _cardBackground->setPosition(actualSize.width * 0.5f, actualSize.height * 0.5f);
        addChild(_cardBackground);

        CCLOG("Card actual size: %.0f x %.0f", actualSize.width, actualSize.height);
    } else {
        // 如果图片加载失败，创建简单的矩形背景
        Size defaultSize = Size(100, 140);
        setContentSize(defaultSize);
        auto drawNode = DrawNode::create();
        drawNode->drawSolidRect(Vec2::ZERO, Vec2(defaultSize.width, defaultSize.height),
                               Color4F::WHITE);
        drawNode->drawRect(Vec2::ZERO, Vec2(defaultSize.width, defaultSize.height),
                          Color4F::BLACK);
        addChild(drawNode);
    }
}

void CardView::createCardFront() {
    _cardFront = Node::create();
    addChild(_cardFront);

    // 创建大数字精灵（中间）- 保持原始尺寸
    _bigNumberSprite = Sprite::create("res/number/big_black_A.png");
    if (_bigNumberSprite) {
        _bigNumberSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        _cardFront->addChild(_bigNumberSprite);
    }

    // 创建小数字精灵（左上角）- 保持原始尺寸
    _smallNumberSprite = Sprite::create("res/number/small_black_A.png");
    if (_smallNumberSprite) {
        _smallNumberSprite->setAnchorPoint(Vec2(0.0f, 1.0f));
        _cardFront->addChild(_smallNumberSprite);
    }

    // 创建花色精灵（右上角）- 保持原始尺寸
    _suitSprite = Sprite::create("res/suits/club.png");
    if (_suitSprite) {
        _suitSprite->setAnchorPoint(Vec2(1.0f, 1.0f));
        _cardFront->addChild(_suitSprite);
    }

    // 立即更新布局（此时背景已经创建完成）
    updateCardLayout();
}

void CardView::createCardBack() {
    _cardBack = Node::create();
    addChild(_cardBack);

    // 创建卡牌背面图案，使用配置中的字体
    auto fontConfig = _configManager->getFontConfig();
    auto cardBackFont = fontConfig->getCardBackFont();

    auto backLabel = Label::createWithSystemFont(
        cardBackFont.text.empty() ? "CARD" : cardBackFont.text,
        cardBackFont.family,
        cardBackFont.size
    );
    backLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
    backLabel->setColor(Color3B::BLUE);

    // 立即设置到中心位置
    Size cardSize = getCardSize();
    backLabel->setPosition(cardSize.width * 0.5f, cardSize.height * 0.5f);

    _cardBack->addChild(backLabel, 0, "back_label");

    // 添加边框
    auto border = createCardBorder();
    if (border) {
        _cardBack->addChild(border);
    }
}

void CardView::updateCardFront() {
    if (!_cardModel || !_bigNumberSprite || !_smallNumberSprite || !_suitSprite) return;

    // 获取牌面文字
    std::string faceText = getFaceText(_cardModel->getFace());

    // 获取颜色（红色或黑色）
    bool isRed = (_cardModel->getSuit() == CST_HEARTS ||
                  _cardModel->getSuit() == CST_DIAMONDS);
    std::string colorPrefix = isRed ? "red" : "black";

    // 更新大数字精灵（中间）
    std::string bigNumberPath = "res/number/big_" + colorPrefix + "_" + faceText + ".png";
    auto bigTexture = Director::getInstance()->getTextureCache()->addImage(bigNumberPath);
    if (bigTexture) {
        _bigNumberSprite->setTexture(bigTexture);
    }

    // 更新小数字精灵（左上角）
    std::string smallNumberPath = "res/number/small_" + colorPrefix + "_" + faceText + ".png";
    auto smallTexture = Director::getInstance()->getTextureCache()->addImage(smallNumberPath);
    if (smallTexture) {
        _smallNumberSprite->setTexture(smallTexture);
    }

    // 更新花色精灵（右上角）
    std::string suitPath = getSuitImagePath(_cardModel->getSuit());
    auto suitTexture = Director::getInstance()->getTextureCache()->addImage(suitPath);
    if (suitTexture) {
        _suitSprite->setTexture(suitTexture);
    }
}

bool CardView::onTouchBegan(Touch* touch, Event* event) {
    CCLOG("CardView::onTouchBegan - Card %s, enabled: %s, animating: %s",
          _cardModel ? _cardModel->toString().c_str() : "null",
          _isEnabled ? "true" : "false",
          _isAnimating ? "true" : "false");
          
    if (!_isEnabled || _isAnimating) {
        CCLOG("CardView::onTouchBegan - Touch rejected (disabled or animating)");
        return false;
    }

    // 检查触摸点是否在卡牌范围内
    Vec2 locationInNode = convertToNodeSpace(touch->getLocation());
    Size cardSize = getCardSize();
    Rect rect = Rect(0, 0, cardSize.width, cardSize.height);

    if (rect.containsPoint(locationInNode)) {
        // 播放按下效果
        playScaleAnimation(0.95f, 0.1f);
        CCLOG("CardView::onTouchBegan - Touch accepted");
        return true;
    }

    CCLOG("CardView::onTouchBegan - Touch outside card bounds");
    return false;
}

void CardView::onTouchEnded(Touch* touch, Event* event) {
    CCLOG("CardView::onTouchEnded - Card %s",
          _cardModel ? _cardModel->toString().c_str() : "null");
          
    // 恢复正常大小
    playScaleAnimation(1.0f, 0.1f);

    // 检查是否仍在卡牌范围内
    Vec2 locationInNode = convertToNodeSpace(touch->getLocation());
    Size cardSize = getCardSize();
    Rect rect = Rect(0, 0, cardSize.width, cardSize.height);

    if (rect.containsPoint(locationInNode)) {
        CCLOG("CardView::onTouchEnded - Triggering click callback");
        // 触发点击回调
        if (_cardClickCallback) {
            _cardClickCallback(this, _cardModel);
        } else {
            CCLOG("CardView::onTouchEnded - No click callback set");
        }
    } else {
        CCLOG("CardView::onTouchEnded - Touch ended outside card bounds");
    }
}

void CardView::onTouchCancelled(Touch* touch, Event* event) {
    // 恢复正常大小
    playScaleAnimation(1.0f, 0.1f);
}

Color3B CardView::getSuitColor(CardSuitType suit) const {
    switch (suit) {
        case CST_HEARTS:
        case CST_DIAMONDS:
            return Color3B::RED;
        case CST_CLUBS:
        case CST_SPADES:
            return Color3B::BLACK;
        default:
            return Color3B::BLACK;
    }
}

Sprite* CardView::createCardBorder() const {
    // 创建简单的边框
    auto drawNode = DrawNode::create();
    Size cardSize = getCardSize();
    drawNode->drawRect(Vec2::ZERO, Vec2(cardSize.width, cardSize.height),
                      Color4F(0, 0, 0, 1));
    return nullptr; // 暂时返回nullptr，后续可以改为返回实际的边框精灵
}

std::string CardView::getFaceText(CardFaceType face) const {
    switch (face) {
        case CFT_ACE:   return "A";
        case CFT_TWO:   return "2";
        case CFT_THREE: return "3";
        case CFT_FOUR:  return "4";
        case CFT_FIVE:  return "5";
        case CFT_SIX:   return "6";
        case CFT_SEVEN: return "7";
        case CFT_EIGHT: return "8";
        case CFT_NINE:  return "9";
        case CFT_TEN:   return "10";
        case CFT_JACK:  return "J";
        case CFT_QUEEN: return "Q";
        case CFT_KING:  return "K";
        default:        return "A";
    }
}

std::string CardView::getSuitImagePath(CardSuitType suit) const {
    switch (suit) {
        case CST_CLUBS:    return "res/suits/club.png";
        case CST_DIAMONDS: return "res/suits/diamond.png";
        case CST_HEARTS:   return "res/suits/heart.png";
        case CST_SPADES:   return "res/suits/spade.png";
        default:           return "res/suits/club.png";
    }
}
