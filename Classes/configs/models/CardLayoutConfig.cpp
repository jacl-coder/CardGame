#include "CardLayoutConfig.h"

CardLayoutConfig::CardLayoutConfig() {
    resetToDefault();
}

CardLayoutConfig::~CardLayoutConfig() {
}

void CardLayoutConfig::resetToDefault() {
    // 默认卡牌布局配置（原硬编码值转换为相对坐标）
    // 原值：Vec2(cardSize.width * 0.5f, cardSize.height * 0.5f)
    _bigNumberPosition = RelativePosition(0.5f, 0.5f);
    
    // 原值：Vec2(cardSize.width * 0.08f, cardSize.height * 0.95f)
    _smallNumberPosition = RelativePosition(0.08f, 0.95f);
    
    // 原值：Vec2(cardSize.width * 0.92f, cardSize.height * 0.95f)
    _suitPosition = RelativePosition(0.92f, 0.95f);
    
    // 卡牌背面文字位置（中心）
    _cardBackTextPosition = RelativePosition(0.5f, 0.5f);
}

bool CardLayoutConfig::fromJson(const rapidjson::Value& json) {
    if (!json.IsObject()) {
        CCLOG("CardLayoutConfig::fromJson - Invalid JSON format");
        return false;
    }
    
    // 解析卡牌布局配置
    if (json.HasMember("CardLayout") && json["CardLayout"].IsObject()) {
        const rapidjson::Value& cardLayout = json["CardLayout"];
        
        if (cardLayout.HasMember("BigNumberPosition") && cardLayout["BigNumberPosition"].IsObject()) {
            _bigNumberPosition = parseRelativePositionFromJson(cardLayout["BigNumberPosition"]);
        }
        
        if (cardLayout.HasMember("SmallNumberPosition") && cardLayout["SmallNumberPosition"].IsObject()) {
            _smallNumberPosition = parseRelativePositionFromJson(cardLayout["SmallNumberPosition"]);
        }
        
        if (cardLayout.HasMember("SuitPosition") && cardLayout["SuitPosition"].IsObject()) {
            _suitPosition = parseRelativePositionFromJson(cardLayout["SuitPosition"]);
        }
        
        if (cardLayout.HasMember("CardBackTextPosition") && cardLayout["CardBackTextPosition"].IsObject()) {
            _cardBackTextPosition = parseRelativePositionFromJson(cardLayout["CardBackTextPosition"]);
        }
    }
    
    return isValid();
}

rapidjson::Value CardLayoutConfig::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value configJson(rapidjson::kObjectType);
    
    // 序列化卡牌布局配置
    rapidjson::Value cardLayoutJson(rapidjson::kObjectType);
    cardLayoutJson.AddMember("BigNumberPosition", serializeRelativePositionToJson(_bigNumberPosition, allocator), allocator);
    cardLayoutJson.AddMember("SmallNumberPosition", serializeRelativePositionToJson(_smallNumberPosition, allocator), allocator);
    cardLayoutJson.AddMember("SuitPosition", serializeRelativePositionToJson(_suitPosition, allocator), allocator);
    cardLayoutJson.AddMember("CardBackTextPosition", serializeRelativePositionToJson(_cardBackTextPosition, allocator), allocator);
    configJson.AddMember("CardLayout", cardLayoutJson, allocator);
    
    return configJson;
}

bool CardLayoutConfig::isValid() const {
    return isValidRelativePosition(_bigNumberPosition) &&
           isValidRelativePosition(_smallNumberPosition) &&
           isValidRelativePosition(_suitPosition) &&
           isValidRelativePosition(_cardBackTextPosition);
}

std::string CardLayoutConfig::getSummary() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "CardLayout - BigNum:(%.2f,%.2f) SmallNum:(%.2f,%.2f) Suit:(%.2f,%.2f)",
             _bigNumberPosition.x, _bigNumberPosition.y,
             _smallNumberPosition.x, _smallNumberPosition.y,
             _suitPosition.x, _suitPosition.y);
    return std::string(buffer);
}

CardLayoutConfig::RelativePosition CardLayoutConfig::parseRelativePositionFromJson(const rapidjson::Value& json) const {
    RelativePosition position;
    
    if (json.HasMember("x") && json["x"].IsNumber()) {
        position.x = json["x"].GetFloat();
    }
    
    if (json.HasMember("y") && json["y"].IsNumber()) {
        position.y = json["y"].GetFloat();
    }
    
    return position;
}

rapidjson::Value CardLayoutConfig::serializeRelativePositionToJson(const RelativePosition& position, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value positionJson(rapidjson::kObjectType);
    positionJson.AddMember("x", position.x, allocator);
    positionJson.AddMember("y", position.y, allocator);
    return positionJson;
}

bool CardLayoutConfig::isValidRelativePosition(const RelativePosition& position) const {
    // 相对坐标应该在0.0到1.0之间
    return (position.x >= 0.0f && position.x <= 1.0f &&
            position.y >= 0.0f && position.y <= 1.0f);
}
