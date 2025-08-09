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

#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

class GameView;
class GameController;

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);

private:
    /**
     * 测试配置系统
     */
    void testConfigSystem();

    /**
     * 创建游戏场景
     */
    void createGameScene();

    /**
     * 初始化关卡选择UI
     */
    void initLevelSelectUI();

    /**
     * 开始指定关卡
     */
    void startLevel(int levelId);

    /**
     * 初始化返回按钮（回到关卡选择）
     */
    void initBackButtonUI();

    /**
     * 返回关卡选择界面，清理已加载的游戏
     */
    void returnToLevelSelect();

private:
    GameView* _gameView = nullptr;
    GameController* _gameController = nullptr;
    cocos2d::Menu* _levelMenu = nullptr;
    cocos2d::Menu* _backMenu = nullptr;
    cocos2d::LayerColor* _levelSelectBg = nullptr;
};

#endif // __GAME_SCENE_H__
