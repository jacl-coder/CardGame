#include "UndoModel.h"
#include <chrono>

UndoModel::UndoModel(UndoOperationType operationType)
    : _operationType(operationType)
    , _sourceCard(nullptr)
    , _targetCard(nullptr)
    , _sourcePosition(Vec2::ZERO)
    , _targetPosition(Vec2::ZERO)
    , _sourceFlippedState(true)
    , _targetFlippedState(true)
    , _scoreDelta(0)
    , _sourceZOrder(0)
    , _timestamp(getCurrentTimestamp()) {
}

UndoModel::~UndoModel() {
}

std::shared_ptr<UndoModel> UndoModel::createStackToCurrentAction(
    std::shared_ptr<CardModel> sourceCard,
    std::shared_ptr<CardModel> targetCard,
    const Vec2& sourcePos,
    const Vec2& targetPos,
    int scoreDelta) {
    
    auto undoAction = std::make_shared<UndoModel>(UndoOperationType::STACK_OPERATION);
    undoAction->setSourceCard(sourceCard);
    undoAction->setTargetCard(targetCard);
    undoAction->setSourcePosition(sourcePos);
    undoAction->setTargetPosition(targetPos);
    undoAction->setScoreDelta(scoreDelta);
    
    if (sourceCard) {
        undoAction->setSourceFlippedState(sourceCard->isFlipped());
        CCLOG("UndoModel::createStackToCurrentAction - Source card: %s at (%.0f, %.0f), flipped: %s",
              sourceCard->toString().c_str(), sourcePos.x, sourcePos.y, 
              sourceCard->isFlipped() ? "true" : "false");
    }
    if (targetCard) {
        undoAction->setTargetFlippedState(targetCard->isFlipped());
        CCLOG("UndoModel::createStackToCurrentAction - Target card: %s at (%.0f, %.0f), flipped: %s",
              targetCard->toString().c_str(), targetPos.x, targetPos.y,
              targetCard->isFlipped() ? "true" : "false");
    }
    
    CCLOG("UndoModel::createStackToCurrentAction - Created STACK_OPERATION undo record, score delta: %d", scoreDelta);
    return undoAction;
}

std::shared_ptr<UndoModel> UndoModel::createPlayfieldToCurrentAction(
    std::shared_ptr<CardModel> sourceCard,
    std::shared_ptr<CardModel> targetCard,
    const Vec2& sourcePos,
    const Vec2& targetPos,
    int scoreDelta,
    int sourceZOrder) {
    
    auto undoAction = std::make_shared<UndoModel>(UndoOperationType::CARD_MOVE);
    undoAction->setSourceCard(sourceCard);
    undoAction->setTargetCard(targetCard);
    undoAction->setSourcePosition(sourcePos);
    undoAction->setTargetPosition(targetPos);
    undoAction->setScoreDelta(scoreDelta);
    undoAction->setSourceZOrder(sourceZOrder);
    
    if (sourceCard) {
        undoAction->setSourceFlippedState(sourceCard->isFlipped());
        CCLOG("UndoModel::createPlayfieldToCurrentAction - Source card: %s at (%.0f, %.0f), flipped: %s",
              sourceCard->toString().c_str(), sourcePos.x, sourcePos.y,
              sourceCard->isFlipped() ? "true" : "false");
    }
    if (targetCard) {
        undoAction->setTargetFlippedState(targetCard->isFlipped());
        CCLOG("UndoModel::createPlayfieldToCurrentAction - Target card: %s at (%.0f, %.0f), flipped: %s",
              targetCard->toString().c_str(), targetPos.x, targetPos.y,
              targetCard->isFlipped() ? "true" : "false");
    }
    
    CCLOG("UndoModel::createPlayfieldToCurrentAction - Created CARD_MOVE undo record, score delta: %d", scoreDelta);
    return undoAction;
}

std::shared_ptr<UndoModel> UndoModel::createFlipCardAction(
    std::shared_ptr<CardModel> card,
    bool originalFlippedState) {
    
    auto undoAction = std::make_shared<UndoModel>(UndoOperationType::CARD_FLIP);
    undoAction->setSourceCard(card);
    undoAction->setSourceFlippedState(originalFlippedState);
    
    if (card) {
        undoAction->setSourcePosition(card->getPosition());
    }
    
    return undoAction;
}

std::string UndoModel::getActionDescription() const {
    switch (_operationType) {
        case UndoOperationType::STACK_OPERATION:
            return "手牌堆到底牌";
        case UndoOperationType::CARD_MOVE:
            return "桌面牌到底牌";
        case UndoOperationType::CARD_FLIP:
            return "翻牌操作";
        default:
            return "未知操作";
    }
}

rapidjson::Value UndoModel::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value undoJson(rapidjson::kObjectType);
    
    undoJson.AddMember("OperationType", static_cast<int>(_operationType), allocator);
    undoJson.AddMember("ScoreDelta", _scoreDelta, allocator);
    undoJson.AddMember("Timestamp", _timestamp, allocator);
    undoJson.AddMember("SourceFlippedState", _sourceFlippedState, allocator);
    undoJson.AddMember("TargetFlippedState", _targetFlippedState, allocator);
    
    // 序列化位置
    rapidjson::Value sourcePosJson(rapidjson::kObjectType);
    sourcePosJson.AddMember("x", _sourcePosition.x, allocator);
    sourcePosJson.AddMember("y", _sourcePosition.y, allocator);
    undoJson.AddMember("SourcePosition", sourcePosJson, allocator);
    
    rapidjson::Value targetPosJson(rapidjson::kObjectType);
    targetPosJson.AddMember("x", _targetPosition.x, allocator);
    targetPosJson.AddMember("y", _targetPosition.y, allocator);
    undoJson.AddMember("TargetPosition", targetPosJson, allocator);
    
    // 序列化卡牌
    if (_sourceCard) {
        undoJson.AddMember("SourceCard", _sourceCard->toJson(allocator), allocator);
    }
    if (_targetCard) {
        undoJson.AddMember("TargetCard", _targetCard->toJson(allocator), allocator);
    }
    
    return undoJson;
}

void UndoModel::fromJson(const rapidjson::Value& json) {
    if (json.HasMember("OperationType") && json["OperationType"].IsInt()) {
        _operationType = static_cast<UndoOperationType>(json["OperationType"].GetInt());
    }
    
    if (json.HasMember("ScoreDelta") && json["ScoreDelta"].IsInt()) {
        _scoreDelta = json["ScoreDelta"].GetInt();
    }
    
    if (json.HasMember("Timestamp") && json["Timestamp"].IsInt64()) {
        _timestamp = json["Timestamp"].GetInt64();
    }
    
    if (json.HasMember("SourceFlippedState") && json["SourceFlippedState"].IsBool()) {
        _sourceFlippedState = json["SourceFlippedState"].GetBool();
    }
    
    if (json.HasMember("TargetFlippedState") && json["TargetFlippedState"].IsBool()) {
        _targetFlippedState = json["TargetFlippedState"].GetBool();
    }
    
    // 反序列化位置
    if (json.HasMember("SourcePosition") && json["SourcePosition"].IsObject()) {
        const rapidjson::Value& pos = json["SourcePosition"];
        if (pos.HasMember("x") && pos.HasMember("y")) {
            _sourcePosition.x = pos["x"].GetFloat();
            _sourcePosition.y = pos["y"].GetFloat();
        }
    }
    
    if (json.HasMember("TargetPosition") && json["TargetPosition"].IsObject()) {
        const rapidjson::Value& pos = json["TargetPosition"];
        if (pos.HasMember("x") && pos.HasMember("y")) {
            _targetPosition.x = pos["x"].GetFloat();
            _targetPosition.y = pos["y"].GetFloat();
        }
    }
    
    // 反序列化卡牌
    if (json.HasMember("SourceCard") && json["SourceCard"].IsObject()) {
        _sourceCard = std::make_shared<CardModel>();
        _sourceCard->fromJson(json["SourceCard"]);
    }
    
    if (json.HasMember("TargetCard") && json["TargetCard"].IsObject()) {
        _targetCard = std::make_shared<CardModel>();
        _targetCard->fromJson(json["TargetCard"]);
    }
}

long long UndoModel::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return timestamp.count();
}
