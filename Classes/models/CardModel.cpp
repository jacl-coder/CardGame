#include "CardModel.h"

// 静态成员初始化
int CardModel::s_nextCardId = 1;

CardModel::CardModel(CardFaceType face, CardSuitType suit, const Vec2& position)
    : _face(face)
    , _suit(suit)
    , _position(position)
    , _cardId(generateCardId())
    , _isFlipped(true) {
}

CardModel::CardModel()
    : _face(CFT_ACE)
    , _suit(CST_CLUBS)
    , _position(Vec2::ZERO)
    , _cardId(generateCardId())
    , _isFlipped(true) {
}

CardModel::~CardModel() {
}

int CardModel::getCardValue() const {
    return static_cast<int>(_face) + 1; // A=1, 2=2, ..., K=13
}

bool CardModel::canMatchWith(const CardModel& other) const {
    int thisValue = getCardValue();
    int otherValue = other.getCardValue();
    
    // 数字相差1即可匹配，无花色限制
    return abs(thisValue - otherValue) == 1;
}

std::string CardModel::toString() const {
    return getSuitSymbol(_suit) + getFaceSymbol(_face);
}

rapidjson::Value CardModel::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value cardJson(rapidjson::kObjectType);
    
    cardJson.AddMember("CardFace", static_cast<int>(_face), allocator);
    cardJson.AddMember("CardSuit", static_cast<int>(_suit), allocator);
    
    rapidjson::Value positionJson(rapidjson::kObjectType);
    positionJson.AddMember("x", _position.x, allocator);
    positionJson.AddMember("y", _position.y, allocator);
    cardJson.AddMember("Position", positionJson, allocator);
    
    cardJson.AddMember("CardId", _cardId, allocator);
    cardJson.AddMember("IsFlipped", _isFlipped, allocator);
    
    return cardJson;
}

void CardModel::fromJson(const rapidjson::Value& json) {
    if (json.HasMember("CardFace") && json["CardFace"].IsInt()) {
        _face = static_cast<CardFaceType>(json["CardFace"].GetInt());
    }
    
    if (json.HasMember("CardSuit") && json["CardSuit"].IsInt()) {
        _suit = static_cast<CardSuitType>(json["CardSuit"].GetInt());
    }
    
    if (json.HasMember("Position") && json["Position"].IsObject()) {
        const rapidjson::Value& pos = json["Position"];
        if (pos.HasMember("x") && pos.HasMember("y")) {
            _position.x = pos["x"].GetFloat();
            _position.y = pos["y"].GetFloat();
        }
    }
    
    if (json.HasMember("CardId") && json["CardId"].IsInt()) {
        _cardId = json["CardId"].GetInt();
        // 更新静态ID计数器
        if (_cardId >= s_nextCardId) {
            s_nextCardId = _cardId + 1;
        }
    }
    
    if (json.HasMember("IsFlipped") && json["IsFlipped"].IsBool()) {
        _isFlipped = json["IsFlipped"].GetBool();
    }
}

int CardModel::generateCardId() {
    return s_nextCardId++;
}

std::string CardModel::getSuitSymbol(CardSuitType suit) const {
    switch (suit) {
        case CST_CLUBS:    return "♣";
        case CST_DIAMONDS: return "♦";
        case CST_HEARTS:   return "♥";
        case CST_SPADES:   return "♠";
        default:           return "?";
    }
}

std::string CardModel::getFaceSymbol(CardFaceType face) const {
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
        default:        return "?";
    }
}
