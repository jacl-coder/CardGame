/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "GameScene.h"
#include "SimpleAudioEngine.h"
#include "models/CardModel.h"
#include "views/CardView.h"
#include "configs/loaders/LevelConfigLoader.h"
#include "views/GameView.h"
#include "controllers/GameController.h"

USING_NS_CC;

Scene* GameScene::createScene()
{
    return GameScene::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in GameScene.cpp\n");
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // 注释掉退出按钮 - 移除退出功能
    /*
    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(GameScene::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    */

    /////////////////////////////
    // 3. add your codes below...

    // 测试配置系统（可选）
    // testConfigSystem(); // 取消注释以查看配置系统测试

    // 创建实际的游戏场景
    createGameScene();

    // 注释掉测试卡牌，只显示游戏场景
    /*
    // 创建测试卡牌 - 使用更好的布局（保留作为对比）
    auto cardModel1 = std::make_shared<CardModel>(CardFaceType::ACE, CardSuitType::HEARTS,
                                                 Vec2(200, 600));
    auto cardView1 = CardView::create(cardModel1);
    cardView1->setCardClickCallback([](CardView* cardView, std::shared_ptr<CardModel> cardModel) {
        CCLOG("Clicked card: %s", cardModel->toString().c_str());
        // 测试翻牌动画
        cardView->setFlipped(!cardView->isFlipped(), true);
    });
    this->addChild(cardView1);

    auto cardModel2 = std::make_shared<CardModel>(CardFaceType::KING, CardSuitType::SPADES,
                                                 Vec2(350, 600));
    auto cardView2 = CardView::create(cardModel2);
    cardView2->setCardClickCallback([cardView1](CardView* cardView, std::shared_ptr<CardModel> cardModel) {
        CCLOG("Clicked card: %s", cardModel->toString().c_str());
        // 测试移动动画
        cardView->playMoveAnimation(Vec2(500, 700), 0.5f, []() {
            CCLOG("Move animation completed!");
        });
    });
    this->addChild(cardView2);

    auto cardModel3 = std::make_shared<CardModel>(CardFaceType::FOUR, CardSuitType::CLUBS,
                                                 Vec2(500, 600));
    auto cardView3 = CardView::create(cardModel3);
    cardView3->setCardClickCallback([](CardView* cardView, std::shared_ptr<CardModel> cardModel) {
        CCLOG("Clicked card: %s", cardModel->toString().c_str());
        // 测试高亮效果
        cardView->setHighlighted(!cardView->isHighlighted());
    });
    this->addChild(cardView3);

    // 添加一张方块Q测试红色卡牌
    auto cardModel4 = std::make_shared<CardModel>(CardFaceType::QUEEN, CardSuitType::DIAMONDS,
                                                 Vec2(650, 600));
    auto cardView4 = CardView::create(cardModel4);
    cardView4->setCardClickCallback([](CardView* cardView, std::shared_ptr<CardModel> cardModel) {
        CCLOG("Clicked card: %s", cardModel->toString().c_str());
        // 测试缩放动画
        cardView->playScaleAnimation(1.2f, 0.2f);
    });
    this->addChild(cardView4);
    */

    // 隐藏测试说明标签，游戏场景有自己的标题
    /*
    // 添加说明标签
    auto label = Label::createWithTTF("CardGame - Real Card Graphics Test\n♥A: Flip | ♠K: Move | ♣4: Highlight | ♦Q: Scale",
                                     "fonts/Marker Felt.ttf", 18);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                               origin.y + visibleSize.height - 100));
        this->addChild(label, 1);
    }
    */
    return true;
}

void GameScene::testConfigSystem() {
    CCLOG("=== Testing Config System ===");

    // 创建配置加载器
    LevelConfigLoader loader;

    // 首先尝试从文件加载关卡1
    CCLOG("🔄 Trying to load level 1 from file...");
    auto config = loader.loadLevelConfig(1);

    if (!config) {
        CCLOG("📁 File not found, level 1 config is missing!");
        // 如果文件加载失败，返回错误
        return;
    }

    // 测试配置
    auto defaultConfig = config;
    if (defaultConfig) {
        CCLOG("✅ Default config created successfully: %s", defaultConfig->getSummary().c_str());

        // 测试配置数据
        const auto& playfieldCards = defaultConfig->getPlayfieldCards();
        const auto& stackCards = defaultConfig->getStackCards();

        CCLOG("📋 Playfield cards: %zu", playfieldCards.size());
        for (size_t i = 0; i < playfieldCards.size(); i++) {
            const auto& card = playfieldCards[i];
            CCLOG("  Card %zu: Face=%d, Suit=%d, Pos=(%.0f,%.0f)",
                  i, static_cast<int>(card.cardFace), static_cast<int>(card.cardSuit),
                  card.position.x, card.position.y);
        }

        CCLOG("🃏 Stack cards: %zu", stackCards.size());
        for (size_t i = 0; i < stackCards.size(); i++) {
            const auto& card = stackCards[i];
            CCLOG("  Card %zu: Face=%d, Suit=%d",
                  i, static_cast<int>(card.cardFace), static_cast<int>(card.cardSuit));
        }

        // 测试配置验证
        if (defaultConfig->isValid()) {
            CCLOG("✅ Config validation passed");
        } else {
            CCLOG("❌ Config validation failed");
        }

    } else {
        CCLOG("❌ Failed to create default config");
    }

    // 测试预加载功能
    CCLOG("🔄 Testing preload functionality...");
    int loadedCount = loader.preloadAllLevelConfigs();
    CCLOG("📦 Preloaded %d levels", loadedCount);

    // 显示所有已加载的关卡
    auto levelIds = loader.getLoadedLevelIds();
    CCLOG("📋 Loaded level IDs:");
    for (int levelId : levelIds) {
        auto cachedConfig = loader.getCachedLevelConfig(levelId);
        if (cachedConfig) {
            CCLOG("  Level %d: %s", levelId, cachedConfig->getSummary().c_str());
        }
    }

    CCLOG("=== Config System Test Complete ===");
}

void GameScene::createGameScene() {
    CCLOG("=== Creating Game Scene ===");

    // 创建游戏视图
    auto gameView = GameView::create();
    if (!gameView) {
        CCLOG("❌ Failed to create game view");
        return;
    }

    // 添加游戏视图到场景
    this->addChild(gameView, 10); // 高层级，显示在测试卡牌之上

    // 创建游戏控制器
    auto gameController = new GameController();
    if (!gameController->init(gameView)) {
        CCLOG("❌ Failed to initialize game controller");
        delete gameController;
        return;
    }

    // 开始游戏（关卡1）
    if (gameController->startGame(1)) {
        CCLOG("✅ Game scene created and started successfully");
        CCLOG("📋 Current game state: %d", static_cast<int>(gameController->getCurrentGameState()));
        CCLOG("🎮 Current level: %d", gameController->getCurrentLevelId());

        // 保存控制器引用（在实际项目中应该有更好的生命周期管理）
        gameView->setUserData(gameController);
    } else {
        CCLOG("❌ Failed to start game");
        delete gameController;
    }

    CCLOG("=== Game Scene Creation Complete ===");
}


void GameScene::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
