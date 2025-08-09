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


    // 初始化关卡选择UI（选择后再创建与加载游戏）
    initLevelSelectUI();
    initBackButtonUI();

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
    CCLOG("=== Preparing Game Scene (deferred load) ===");
}

void GameScene::initLevelSelectUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 关卡选择背景（避免纯黑）
    if (_levelSelectBg == nullptr) {
        // 渐变或纯色，使用柔和的蓝绿色调
        Color4B bgColor(80, 39, 97, 255);   //rgb(80, 39, 97)
        _levelSelectBg = LayerColor::create(bgColor);
        _levelSelectBg->setContentSize(visibleSize);
        _levelSelectBg->setPosition(origin);
        this->addChild(_levelSelectBg, 5);
    } else {
        _levelSelectBg->setVisible(true);
    }

    // 列出关卡目录中的所有关卡文件
    std::string levelsDir = "configs/data/levels/";
    std::vector<std::string> files = FileUtils::getInstance()->listFiles(levelsDir);

    std::vector<int> levelIds;
    levelIds.reserve(files.size());

    auto extractLevelId = [](const std::string& filename) -> int {
        // 期望格式: level_<id>.json
        size_t slash = filename.find_last_of("/");
        std::string name = (slash == std::string::npos) ? filename : filename.substr(slash + 1);
        if (name.size() < 12) return -1; // 最小长度粗略检查
        const std::string prefix = "level_";
        const std::string suffix = ".json";
        if (name.rfind(prefix, 0) != 0) return -1; // 必须以 level_ 开头
        if (name.size() <= prefix.size() + suffix.size()) return -1;
        if (name.substr(name.size() - suffix.size()) != suffix) return -1;
        std::string idPart = name.substr(prefix.size(), name.size() - prefix.size() - suffix.size());
        for (char c : idPart) { if (c < '0' || c > '9') return -1; }
        return idPart.empty() ? -1 : atoi(idPart.c_str());
    };

    for (const auto& path : files) {
        // 过滤目录，仅保留文件
        if (FileUtils::getInstance()->isDirectoryExist(path)) continue;
        int id = extractLevelId(path);
        if (id > 0) levelIds.push_back(id);
    }

    if (levelIds.empty()) {
        CCLOG("initLevelSelectUI - No level files found in %s", levelsDir.c_str());
        // 仍然显示一个占位按钮，便于调试
        levelIds.push_back(1);
    }

    std::sort(levelIds.begin(), levelIds.end());
    levelIds.erase(std::unique(levelIds.begin(), levelIds.end()), levelIds.end());

    // 创建菜单项，按列排列（居中，竖向）
    std::vector<MenuItem*> items;
    items.reserve(levelIds.size());

    const float fontSize = 48.0f;
    const float gapY = 150.0f; // 拉开间距
    const float centerX = origin.x + visibleSize.width * 0.5f;
    const float totalHeight = (levelIds.empty() ? 0.0f : (static_cast<float>(levelIds.size() - 1) * gapY));
    const float startY = origin.y + visibleSize.height * 0.5f + totalHeight * 0.5f + 20.0f; // 略微上移

    for (size_t i = 0; i < levelIds.size(); ++i) {
        int levelId = levelIds[i];
        auto label = Label::createWithTTF(StringUtils::format("Level %d", levelId), "fonts/Marker Felt.ttf", fontSize);
        label->setColor(Color3B::WHITE);

        // 计算按钮尺寸（根据文字大小添加内边距）
        const Size textSize = label->getContentSize();
        const float paddingX = 28.0f;
        const float paddingY = 22.0f;
        const Size btnSize(textSize.width + paddingX * 2.0f, textSize.height + paddingY * 2.0f);

        auto makeButtonNode = [&](const Color4F& bgColor, const Color4F& borderColor) -> Node* {
            auto container = Node::create();
            container->setContentSize(btnSize);
            // 背景
            auto bg = DrawNode::create();
            bg->drawSolidRect(Vec2::ZERO, Vec2(btnSize.width, btnSize.height), bgColor);
            // 边框
            bg->drawRect(Vec2(0.5f, 0.5f), Vec2(btnSize.width - 0.5f, btnSize.height - 0.5f), borderColor);
            container->addChild(bg);
            // 文本
            auto text = Label::createWithTTF(StringUtils::format("Level %d", levelId), "fonts/Marker Felt.ttf", fontSize);
            text->setColor(Color3B::WHITE);
            text->setPosition(Vec2(btnSize.width * 0.5f, btnSize.height * 0.5f));
            container->addChild(text, 1);
            return container;
        };

        // 正常与按下态颜色
        // 更明亮的按钮配色
        Color4F normalBg(0.22f, 0.36f, 0.52f, 0.96f);     // 蓝色
        Color4F normalBorder(1.f, 1.f, 1.f, 0.60f);
        Color4F selectedBg(0.30f, 0.50f, 0.70f, 0.98f);   // 更亮
        Color4F selectedBorder(1.f, 1.f, 1.f, 0.80f);

        auto normalNode = makeButtonNode(normalBg, normalBorder);
        auto selectedNode = makeButtonNode(selectedBg, selectedBorder);

        auto item = MenuItemSprite::create(normalNode, selectedNode, nullptr, [this, levelId](Ref*) {
            this->startLevel(levelId);
        });
        item->setPosition(Vec2(centerX, startY - static_cast<float>(i) * gapY));
        items.push_back(item);
    }

    // 如果数量太多，简单地限制在可视范围内（生产可改为滚动列表）
    // 这里只是基础实现

    // 将 std::vector<MenuItem*> 转换为 cocos2d::Vector<MenuItem*>
    cocos2d::Vector<MenuItem*> menuItems;
    menuItems.reserve(static_cast<int>(items.size()));
    for (auto* it : items) {
        menuItems.pushBack(it);
    }

    _levelMenu = Menu::createWithArray(menuItems);
    _levelMenu->setPosition(Vec2::ZERO);
    this->addChild(_levelMenu, 20);
}

void GameScene::startLevel(int levelId) {
    CCLOG("startLevel - Starting level %d", levelId);

    // 若已有游戏在运行，则先彻底清理
    if (_gameController || _gameView) {
        CCLOG("startLevel - Cleaning previous game before starting new level");
        if (_gameController) {
            delete _gameController;
            _gameController = nullptr;
        }
        if (_gameView) {
            _gameView->removeFromParentAndCleanup(true);
            _gameView = nullptr;
        }
    }

    // 首次选择关卡时创建视图与控制器
    if (_gameView == nullptr) {
        _gameView = GameView::create();
        if (!_gameView) {
            CCLOG("❌ Failed to create game view");
            return;
        }
        this->addChild(_gameView, 10);
    }

    if (_gameController == nullptr) {
        _gameController = new GameController();
        if (!_gameController->init(_gameView)) {
            CCLOG("❌ Failed to initialize game controller");
            delete _gameController;
            _gameController = nullptr;
            return;
        }
    }

    // 开始关卡
    if (_gameController->startGame(levelId)) {
        CCLOG("✅ Level %d started", levelId);
        _gameView->setUserData(_gameController);
        if (_levelMenu) _levelMenu->setVisible(false); // 隐藏关卡菜单
        if (_levelSelectBg) _levelSelectBg->setVisible(false); // 隐藏背景
        if (_backMenu) _backMenu->setVisible(true);    // 显示返回按钮
    } else {
        CCLOG("❌ Failed to start level %d", levelId);
    }
}

void GameScene::initBackButtonUI() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto label = Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36);
    auto backItem = MenuItemLabel::create(label, [this](Ref*) {
        this->returnToLevelSelect();
    });
    // 放在右上角
    backItem->setPosition(Vec2(origin.x + visibleSize.width - 60.0f,
                               origin.y + visibleSize.height - 80.0f));

    _backMenu = Menu::create(backItem, nullptr);
    _backMenu->setPosition(Vec2::ZERO);
    _backMenu->setVisible(false); // 初始隐藏，进入关卡后显示
    this->addChild(_backMenu, 21);
}

void GameScene::returnToLevelSelect() {
    CCLOG("returnToLevelSelect - Returning to level selection");

    // 彻底销毁并回到初始状态，避免悬垂指针
    if (_gameController) {
        delete _gameController;
        _gameController = nullptr;
    }
    if (_gameView) {
        _gameView->removeFromParentAndCleanup(true);
        _gameView = nullptr;
    }

    // 切换UI可见性
    if (_levelMenu) _levelMenu->setVisible(true);
    if (_levelSelectBg) _levelSelectBg->setVisible(true);
    if (_backMenu) _backMenu->setVisible(false);
}


void GameScene::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
