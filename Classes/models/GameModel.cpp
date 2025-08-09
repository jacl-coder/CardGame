#include "GameModel.h"
#include "UndoModel.h"

GameModel::GameModel()
    : _gameState(GameState::INITIALIZING)
    , _currentCard(nullptr)
    , _score(0)
    , _moveCount(0)
    , _currentLevel(1) {
}

GameModel::~GameModel() {
    clearPlayfieldCards();
    clearStackCards();
}

void GameModel::addPlayfieldCard(std::shared_ptr<CardModel> card) {
    if (card) {
        _playfieldCards.push_back(card);
    }
}

void GameModel::removePlayfieldCard(int cardId) {
    auto it = std::find_if(_playfieldCards.begin(), _playfieldCards.end(),
        [cardId](const std::shared_ptr<CardModel>& card) {
            return card && card->getCardId() == cardId;
        });
    
    if (it != _playfieldCards.end()) {
        _playfieldCards.erase(it);
    }
}

std::shared_ptr<CardModel> GameModel::getPlayfieldCard(int cardId) const {
    auto it = std::find_if(_playfieldCards.begin(), _playfieldCards.end(),
        [cardId](const std::shared_ptr<CardModel>& card) {
            return card && card->getCardId() == cardId;
        });
    
    return (it != _playfieldCards.end()) ? *it : nullptr;
}

void GameModel::clearPlayfieldCards() {
    _playfieldCards.clear();
}

void GameModel::addStackCard(std::shared_ptr<CardModel> card) {
    if (card) {
        _stackCards.push_back(card);
    }
}

std::shared_ptr<CardModel> GameModel::removeTopStackCard() {
    if (_stackCards.empty()) {
        return nullptr;
    }
    
    auto topCard = _stackCards.back();
    _stackCards.pop_back();
    return topCard;
}

std::shared_ptr<CardModel> GameModel::getTopStackCard() const {
    return _stackCards.empty() ? nullptr : _stackCards.back();
}

void GameModel::clearStackCards() {
    _stackCards.clear();
}

void GameModel::pushCurrentCard(std::shared_ptr<CardModel> card) {
    if (card) {
        _currentCardStack.push_back(card);
        _currentCard = card; // 更新当前底牌为栈顶
    }
}

std::shared_ptr<CardModel> GameModel::popCurrentCard() {
    if (_currentCardStack.empty()) {
        return nullptr;
    }
    
    auto topCard = _currentCardStack.back();
    _currentCardStack.pop_back();
    
    // 更新当前底牌为新的栈顶，如果栈为空则为nullptr
    _currentCard = _currentCardStack.empty() ? nullptr : _currentCardStack.back();
    
    return topCard;
}

std::shared_ptr<CardModel> GameModel::peekCurrentCard() const {
    return _currentCardStack.empty() ? nullptr : _currentCardStack.back();
}

void GameModel::clearCurrentCardStack() {
    _currentCardStack.clear();
    _currentCard = nullptr;
}

bool GameModel::hasMatchableCards() const {
    if (!_currentCard) {
        return false;
    }
    
    for (const auto& card : _playfieldCards) {
        if (card && card->isFlipped() && card->canMatchWith(*_currentCard)) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::shared_ptr<CardModel>> GameModel::getMatchableCards() const {
    std::vector<std::shared_ptr<CardModel>> matchableCards;
    
    if (!_currentCard) {
        return matchableCards;
    }
    
    for (const auto& card : _playfieldCards) {
        if (card && card->isFlipped() && card->canMatchWith(*_currentCard)) {
            matchableCards.push_back(card);
        }
    }
    
    return matchableCards;
}

bool GameModel::isGameWon() const {
    // 当桌面没有翻开的卡牌时，游戏胜利
    for (const auto& card : _playfieldCards) {
        if (card && card->isFlipped()) {
            return false;
        }
    }
    return true;
}

void GameModel::resetGame() {
    _gameState = GameState::INITIALIZING;
    clearPlayfieldCards();
    clearStackCards();
    _currentCard = nullptr;
    _score = 0;
    _moveCount = 0;
}

rapidjson::Value GameModel::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value gameJson(rapidjson::kObjectType);
    
    gameJson.AddMember("GameState", static_cast<int>(_gameState), allocator);
    gameJson.AddMember("Score", _score, allocator);
    gameJson.AddMember("MoveCount", _moveCount, allocator);
    gameJson.AddMember("CurrentLevel", _currentLevel, allocator);
    
    // 序列化桌面牌
    gameJson.AddMember("Playfield", serializeCards(_playfieldCards, allocator), allocator);
    
    // 序列化手牌堆
    gameJson.AddMember("Stack", serializeCards(_stackCards, allocator), allocator);
    
    // 序列化当前底牌
    if (_currentCard) {
        gameJson.AddMember("CurrentCard", _currentCard->toJson(allocator), allocator);
    }
    
    return gameJson;
}

void GameModel::fromJson(const rapidjson::Value& json) {
    if (json.HasMember("GameState") && json["GameState"].IsInt()) {
        _gameState = static_cast<GameState>(json["GameState"].GetInt());
    }
    
    if (json.HasMember("Score") && json["Score"].IsInt()) {
        _score = json["Score"].GetInt();
    }
    
    if (json.HasMember("MoveCount") && json["MoveCount"].IsInt()) {
        _moveCount = json["MoveCount"].GetInt();
    }
    
    if (json.HasMember("CurrentLevel") && json["CurrentLevel"].IsInt()) {
        _currentLevel = json["CurrentLevel"].GetInt();
    }
    
    // 反序列化桌面牌
    if (json.HasMember("Playfield") && json["Playfield"].IsArray()) {
        _playfieldCards = deserializeCards(json["Playfield"]);
    }
    
    // 反序列化手牌堆
    if (json.HasMember("Stack") && json["Stack"].IsArray()) {
        _stackCards = deserializeCards(json["Stack"]);
    }
    
    // 反序列化当前底牌
    if (json.HasMember("CurrentCard") && json["CurrentCard"].IsObject()) {
        _currentCard = std::make_shared<CardModel>();
        _currentCard->fromJson(json["CurrentCard"]);
    }
}

rapidjson::Value GameModel::serializeCards(const std::vector<std::shared_ptr<CardModel>>& cards,
                                          rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value cardsArray(rapidjson::kArrayType);
    
    for (const auto& card : cards) {
        if (card) {
            cardsArray.PushBack(card->toJson(allocator), allocator);
        }
    }
    
    return cardsArray;
}

std::vector<std::shared_ptr<CardModel>> GameModel::deserializeCards(const rapidjson::Value& jsonArray) const {
    std::vector<std::shared_ptr<CardModel>> cards;
    
    if (!jsonArray.IsArray()) {
        return cards;
    }
    
    for (rapidjson::SizeType i = 0; i < jsonArray.Size(); i++) {
        if (jsonArray[i].IsObject()) {
            auto card = std::make_shared<CardModel>();
            card->fromJson(jsonArray[i]);
            cards.push_back(card);
        }
    }
    
    return cards;
}

bool GameModel::undoCardMove(std::shared_ptr<UndoModel> undoModel) {
    if (!undoModel) {
        CCLOG("GameModel::undoCardMove - Invalid undo model");
        return false;
    }

    CCLOG("GameModel::undoCardMove - Undoing card move operation");
    
    // 撤销桌面牌到底牌的操作
    auto sourceCard = undoModel->getSourceCard();
    auto targetCard = undoModel->getTargetCard();
    
    if (!sourceCard || !targetCard) {
        CCLOG("GameModel::undoCardMove - Missing source or target card");
        return false;
    }
    
    // 1. 从底牌栈弹出当前牌，恢复为原底牌
    if (_currentCardStack.empty()) {
        CCLOG("GameModel::undoCardMove - Current card stack is empty, cannot undo");
        return false;
    }
    
    auto restoredCard = popCurrentCard();
    if (!restoredCard) {
        CCLOG("GameModel::undoCardMove - Failed to pop current card");
        return false;
    }
    
    CCLOG("GameModel::undoCardMove - Popped card: %s", restoredCard->toString().c_str());
    
    // 关键修复：恢复为UndoModel中记录的原底牌，而不是栈顶
    setCurrentCard(targetCard);
    targetCard->setFlipped(undoModel->getTargetFlippedState());
    
    CCLOG("GameModel::undoCardMove - Restored bottom card: %s", targetCard->toString().c_str());
    
    // 2. 将移动的桌面牌放回原位置（恢复相对位置，不是世界坐标）
    // 注意：undoModel存储的是世界坐标，实际位置恢复将在GameController中处理
    sourceCard->setFlipped(undoModel->getSourceFlippedState());
    
    // 重新添加到桌面卡牌列表
    addPlayfieldCard(sourceCard);
    CCLOG("GameModel::undoCardMove - Re-added card to playfield: %s", sourceCard->toString().c_str());
    
    // 3. 恢复分数和移动次数
    _score -= undoModel->getScoreDelta();
    if (_moveCount > 0) {
        _moveCount--;
    }
    
    CCLOG("GameModel::undoCardMove - Card move undo successful. Score: %d, Move count: %d", 
          _score, _moveCount);
    
    return true;
}

bool GameModel::undoCardFlip(std::shared_ptr<UndoModel> undoModel) {
    if (!undoModel) {
        CCLOG("GameModel::undoCardFlip - Invalid undo model");
        return false;
    }

    // 这里实现卡牌翻转的撤销逻辑
    // 暂时返回true，具体实现将在后续添加
    CCLOG("GameModel::undoCardFlip - Undoing card flip operation");
    return true;
}

bool GameModel::undoStackOperation(std::shared_ptr<UndoModel> undoModel) {
    if (!undoModel) {
        CCLOG("GameModel::undoStackOperation - Invalid undo model");
        return false;
    }

    CCLOG("GameModel::undoStackOperation - Undoing stack operation");
    
    // 撤销手牌堆到底牌的操作
    auto sourceCard = undoModel->getSourceCard();
    auto targetCard = undoModel->getTargetCard();
    
    if (!sourceCard || !targetCard) {
        CCLOG("GameModel::undoStackOperation - Missing source or target card");
        return false;
    }
    
    // 1. 恢复原底牌
    setCurrentCard(targetCard);
    targetCard->setFlipped(undoModel->getTargetFlippedState());
    
    CCLOG("GameModel::undoStackOperation - Restored bottom card: %s", targetCard->toString().c_str());
    
    // 2. 将手牌放回手牌堆顶部
    sourceCard->setFlipped(undoModel->getSourceFlippedState());
    
    // 插入到手牌堆末尾（作为新的栈顶）
    _stackCards.push_back(sourceCard);
    
    CCLOG("GameModel::undoStackOperation - Restored card to stack: %s", sourceCard->toString().c_str());
    CCLOG("GameModel::undoStackOperation - Stack size after restore: %zu", _stackCards.size());
    
    // 3. 恢复分数和移动次数
    _score -= undoModel->getScoreDelta();
    if (_moveCount > 0) {
        _moveCount--;
    }
    
    CCLOG("GameModel::undoStackOperation - Stack operation undo successful. Score: %d, Move count: %d", 
          _score, _moveCount);
    
    return true;
}
