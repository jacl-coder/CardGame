#include "GameRulesConfig.h"

GameRulesConfig::GameRulesConfig() {
    resetToDefault();
}

GameRulesConfig::~GameRulesConfig() {
}

void GameRulesConfig::resetToDefault() {
    // 默认撤销设置（原硬编码值）
    _undoSettings = UndoSettings(10, true);
    
    // 默认卡牌生成设置（原硬编码值）
    _cardGenerationSettings = CardGenerationSettings(1000, false);
    
    // 默认匹配规则设置
    _matchingRules = MatchingRules(true, true, 1);
}

bool GameRulesConfig::fromJson(const rapidjson::Value& json) {
    if (!json.IsObject()) {
        CCLOG("GameRulesConfig::fromJson - Invalid JSON format");
        return false;
    }
    
    // 解析撤销设置
    if (json.HasMember("UndoSettings") && json["UndoSettings"].IsObject()) {
        _undoSettings = parseUndoSettingsFromJson(json["UndoSettings"]);
    }
    
    // 解析卡牌生成设置
    if (json.HasMember("CardGeneration") && json["CardGeneration"].IsObject()) {
        _cardGenerationSettings = parseCardGenerationSettingsFromJson(json["CardGeneration"]);
    }
    
    // 解析匹配规则设置
    if (json.HasMember("MatchingRules") && json["MatchingRules"].IsObject()) {
        _matchingRules = parseMatchingRulesFromJson(json["MatchingRules"]);
    }
    
    return isValid();
}

rapidjson::Value GameRulesConfig::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value configJson(rapidjson::kObjectType);
    
    // 序列化撤销设置
    configJson.AddMember("UndoSettings", serializeUndoSettingsToJson(_undoSettings, allocator), allocator);
    
    // 序列化卡牌生成设置
    configJson.AddMember("CardGeneration", serializeCardGenerationSettingsToJson(_cardGenerationSettings, allocator), allocator);
    
    // 序列化匹配规则设置
    configJson.AddMember("MatchingRules", serializeMatchingRulesToJson(_matchingRules, allocator), allocator);
    
    return configJson;
}

bool GameRulesConfig::isValid() const {
    // 检查撤销设置有效性
    if (_undoSettings.maxUndoSteps < 0 || _undoSettings.maxUndoSteps > 100) {
        return false;
    }
    
    // 检查卡牌生成设置有效性
    if (_cardGenerationSettings.startingCardId < 0) {
        return false;
    }
    
    // 检查匹配规则有效性
    if (_matchingRules.matchDifference < 1 || _matchingRules.matchDifference > 12) {
        return false;
    }
    
    return true;
}

std::string GameRulesConfig::getSummary() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "GameRules - Undo:%d/%s CardGen:%d/%s Match:%d/%s/%s",
             _undoSettings.maxUndoSteps, _undoSettings.enableUndo ? "On" : "Off",
             _cardGenerationSettings.startingCardId, _cardGenerationSettings.shuffleOnLoad ? "Shuffle" : "NoShuffle",
             _matchingRules.matchDifference, _matchingRules.allowCyclicMatching ? "Cyclic" : "NoCyclic",
             _matchingRules.ignoreSuit ? "NoSuit" : "WithSuit");
    return std::string(buffer);
}

GameRulesConfig::UndoSettings GameRulesConfig::parseUndoSettingsFromJson(const rapidjson::Value& json) const {
    UndoSettings settings;
    
    if (json.HasMember("MaxUndoSteps") && json["MaxUndoSteps"].IsInt()) {
        settings.maxUndoSteps = json["MaxUndoSteps"].GetInt();
    }
    
    if (json.HasMember("EnableUndo") && json["EnableUndo"].IsBool()) {
        settings.enableUndo = json["EnableUndo"].GetBool();
    }
    
    return settings;
}

rapidjson::Value GameRulesConfig::serializeUndoSettingsToJson(const UndoSettings& settings, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value settingsJson(rapidjson::kObjectType);
    settingsJson.AddMember("MaxUndoSteps", settings.maxUndoSteps, allocator);
    settingsJson.AddMember("EnableUndo", settings.enableUndo, allocator);
    return settingsJson;
}

GameRulesConfig::CardGenerationSettings GameRulesConfig::parseCardGenerationSettingsFromJson(const rapidjson::Value& json) const {
    CardGenerationSettings settings;
    
    if (json.HasMember("StartingCardId") && json["StartingCardId"].IsInt()) {
        settings.startingCardId = json["StartingCardId"].GetInt();
    }
    
    if (json.HasMember("ShuffleOnLoad") && json["ShuffleOnLoad"].IsBool()) {
        settings.shuffleOnLoad = json["ShuffleOnLoad"].GetBool();
    }
    
    return settings;
}

rapidjson::Value GameRulesConfig::serializeCardGenerationSettingsToJson(const CardGenerationSettings& settings, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value settingsJson(rapidjson::kObjectType);
    settingsJson.AddMember("StartingCardId", settings.startingCardId, allocator);
    settingsJson.AddMember("ShuffleOnLoad", settings.shuffleOnLoad, allocator);
    return settingsJson;
}

GameRulesConfig::MatchingRules GameRulesConfig::parseMatchingRulesFromJson(const rapidjson::Value& json) const {
    MatchingRules rules;
    
    if (json.HasMember("AllowCyclicMatching") && json["AllowCyclicMatching"].IsBool()) {
        rules.allowCyclicMatching = json["AllowCyclicMatching"].GetBool();
    }
    
    if (json.HasMember("IgnoreSuit") && json["IgnoreSuit"].IsBool()) {
        rules.ignoreSuit = json["IgnoreSuit"].GetBool();
    }
    
    if (json.HasMember("MatchDifference") && json["MatchDifference"].IsInt()) {
        rules.matchDifference = json["MatchDifference"].GetInt();
    }
    
    return rules;
}

rapidjson::Value GameRulesConfig::serializeMatchingRulesToJson(const MatchingRules& rules, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value rulesJson(rapidjson::kObjectType);
    rulesJson.AddMember("AllowCyclicMatching", rules.allowCyclicMatching, allocator);
    rulesJson.AddMember("IgnoreSuit", rules.ignoreSuit, allocator);
    rulesJson.AddMember("MatchDifference", rules.matchDifference, allocator);
    return rulesJson;
}
