#include "LevelConfig.h"

// CardConfigData 实现
rapidjson::Value CardConfigData::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value cardJson(rapidjson::kObjectType);
    
    cardJson.AddMember("CardFace", static_cast<int>(cardFace), allocator);
    cardJson.AddMember("CardSuit", static_cast<int>(cardSuit), allocator);
    
    rapidjson::Value positionJson(rapidjson::kObjectType);
    positionJson.AddMember("x", position.x, allocator);
    positionJson.AddMember("y", position.y, allocator);
    cardJson.AddMember("Position", positionJson, allocator);
    
    return cardJson;
}

void CardConfigData::fromJson(const rapidjson::Value& json) {
    if (json.HasMember("CardFace") && json["CardFace"].IsInt()) {
        cardFace = static_cast<CardFaceType>(json["CardFace"].GetInt());
    }
    
    if (json.HasMember("CardSuit") && json["CardSuit"].IsInt()) {
        cardSuit = static_cast<CardSuitType>(json["CardSuit"].GetInt());
    }
    
    if (json.HasMember("Position") && json["Position"].IsObject()) {
        const rapidjson::Value& pos = json["Position"];
        if (pos.HasMember("x") && pos.HasMember("y")) {
            position.x = pos["x"].GetFloat();
            position.y = pos["y"].GetFloat();
        }
    }
}

// LevelConfig 实现
LevelConfig::LevelConfig()
    : _levelId(0)
    , _levelName("")
    , _playfieldSize(Size(1080, 1500))  // 默认主牌区尺寸
    , _stackSize(Size(1080, 580)) {     // 默认堆牌区尺寸
}

LevelConfig::~LevelConfig() {
    reset();
}

void LevelConfig::addPlayfieldCard(const CardConfigData& cardData) {
    _playfieldCards.push_back(cardData);
}

void LevelConfig::clearPlayfieldCards() {
    _playfieldCards.clear();
}

void LevelConfig::addStackCard(const CardConfigData& cardData) {
    _stackCards.push_back(cardData);
}

void LevelConfig::clearStackCards() {
    _stackCards.clear();
}

bool LevelConfig::isValid() const {
    // 检查基本配置
    if (_levelId <= 0) {
        CCLOG("LevelConfig::isValid - Invalid level ID: %d", _levelId);
        return false;
    }
    
    // 检查是否有桌面牌
    if (_playfieldCards.empty()) {
        CCLOG("LevelConfig::isValid - No playfield cards configured");
        return false;
    }
    
    // 检查是否有手牌堆
    if (_stackCards.empty()) {
        CCLOG("LevelConfig::isValid - No stack cards configured");
        return false;
    }
    
    // 检查卡牌数据有效性
    for (const auto& card : _playfieldCards) {
        if (static_cast<int>(card.cardFace) < 0 || static_cast<int>(card.cardFace) >= 13) {
            CCLOG("LevelConfig::isValid - Invalid playfield card face: %d", static_cast<int>(card.cardFace));
            return false;
        }
        if (static_cast<int>(card.cardSuit) < 0 || static_cast<int>(card.cardSuit) >= 4) {
            CCLOG("LevelConfig::isValid - Invalid playfield card suit: %d", static_cast<int>(card.cardSuit));
            return false;
        }
    }
    
    for (const auto& card : _stackCards) {
        if (static_cast<int>(card.cardFace) < 0 || static_cast<int>(card.cardFace) >= 13) {
            CCLOG("LevelConfig::isValid - Invalid stack card face: %d", static_cast<int>(card.cardFace));
            return false;
        }
        if (static_cast<int>(card.cardSuit) < 0 || static_cast<int>(card.cardSuit) >= 4) {
            CCLOG("LevelConfig::isValid - Invalid stack card suit: %d", static_cast<int>(card.cardSuit));
            return false;
        }
    }
    
    return true;
}

std::string LevelConfig::getSummary() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), 
             "Level %d: '%s' - Playfield: %zu cards, Stack: %zu cards", 
             _levelId, _levelName.c_str(), _playfieldCards.size(), _stackCards.size());
    return std::string(buffer);
}

rapidjson::Value LevelConfig::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value levelJson(rapidjson::kObjectType);
    
    levelJson.AddMember("LevelId", _levelId, allocator);
    
    rapidjson::Value nameValue;
    nameValue.SetString(_levelName.c_str(), _levelName.length(), allocator);
    levelJson.AddMember("LevelName", nameValue, allocator);
    
    // 序列化桌面牌区
    levelJson.AddMember("Playfield", serializeCardArray(_playfieldCards, allocator), allocator);
    
    // 序列化手牌堆
    levelJson.AddMember("Stack", serializeCardArray(_stackCards, allocator), allocator);
    
    // 序列化尺寸配置
    rapidjson::Value playfieldSizeJson(rapidjson::kObjectType);
    playfieldSizeJson.AddMember("width", _playfieldSize.width, allocator);
    playfieldSizeJson.AddMember("height", _playfieldSize.height, allocator);
    levelJson.AddMember("PlayfieldSize", playfieldSizeJson, allocator);
    
    rapidjson::Value stackSizeJson(rapidjson::kObjectType);
    stackSizeJson.AddMember("width", _stackSize.width, allocator);
    stackSizeJson.AddMember("height", _stackSize.height, allocator);
    levelJson.AddMember("StackSize", stackSizeJson, allocator);
    
    return levelJson;
}

bool LevelConfig::fromJson(const rapidjson::Value& json) {
    if (!json.IsObject()) {
        CCLOG("LevelConfig::fromJson - Invalid JSON format");
        return false;
    }

    // 解析基本信息
    if (json.HasMember("LevelId") && json["LevelId"].IsInt()) {
        _levelId = json["LevelId"].GetInt();
    }

    if (json.HasMember("LevelName") && json["LevelName"].IsString()) {
        _levelName = json["LevelName"].GetString();
    }

    // 解析桌面牌区
    if (json.HasMember("Playfield") && json["Playfield"].IsArray()) {
        _playfieldCards = deserializeCardArray(json["Playfield"]);
    }

    // 解析手牌堆
    if (json.HasMember("Stack") && json["Stack"].IsArray()) {
        _stackCards = deserializeCardArray(json["Stack"]);
    }

    // 解析尺寸配置
    if (json.HasMember("PlayfieldSize") && json["PlayfieldSize"].IsObject()) {
        const rapidjson::Value& size = json["PlayfieldSize"];
        if (size.HasMember("width") && size.HasMember("height")) {
            _playfieldSize.width = size["width"].GetFloat();
            _playfieldSize.height = size["height"].GetFloat();
        }
    }

    if (json.HasMember("StackSize") && json["StackSize"].IsObject()) {
        const rapidjson::Value& size = json["StackSize"];
        if (size.HasMember("width") && size.HasMember("height")) {
            _stackSize.width = size["width"].GetFloat();
            _stackSize.height = size["height"].GetFloat();
        }
    }

    return isValid();
}

void LevelConfig::reset() {
    _levelId = 0;
    _levelName.clear();
    _playfieldCards.clear();
    _stackCards.clear();
    _playfieldSize = Size(1080, 1500);
    _stackSize = Size(1080, 580);
}

rapidjson::Value LevelConfig::serializeCardArray(const std::vector<CardConfigData>& cards,
                                                rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value cardsArray(rapidjson::kArrayType);

    for (const auto& card : cards) {
        cardsArray.PushBack(card.toJson(allocator), allocator);
    }

    return cardsArray;
}

std::vector<CardConfigData> LevelConfig::deserializeCardArray(const rapidjson::Value& jsonArray) const {
    std::vector<CardConfigData> cards;

    if (!jsonArray.IsArray()) {
        return cards;
    }

    for (rapidjson::SizeType i = 0; i < jsonArray.Size(); i++) {
        if (jsonArray[i].IsObject()) {
            CardConfigData cardData;
            cardData.fromJson(jsonArray[i]);
            cards.push_back(cardData);
        }
    }

    return cards;
}
