#include "FontConfig.h"

FontConfig::FontConfig() {
    resetToDefault();
}

FontConfig::~FontConfig() {
}

void FontConfig::resetToDefault() {
    // 默认卡牌字体配置（原硬编码值）
    _bigNumberFont = FontInfo("Arial", 24.0f);
    _smallNumberFont = FontInfo("Arial", 12.0f);
    _suitFont = FontInfo("Arial", 16.0f);
    _cardBackFont = FontInfo("Arial", 16.0f, "CARD");
    
    // 默认UI字体配置
    _titleFont = FontInfo("Arial", 24.0f);
    _buttonFont = FontInfo("Arial", 18.0f);
}

bool FontConfig::fromJson(const rapidjson::Value& json) {
    if (!json.IsObject()) {
        CCLOG("FontConfig::fromJson - Invalid JSON format");
        return false;
    }
    
    // 解析卡牌字体配置
    if (json.HasMember("CardFonts") && json["CardFonts"].IsObject()) {
        const rapidjson::Value& cardFonts = json["CardFonts"];
        
        if (cardFonts.HasMember("BigNumber") && cardFonts["BigNumber"].IsObject()) {
            _bigNumberFont = parseFontInfoFromJson(cardFonts["BigNumber"]);
        }
        
        if (cardFonts.HasMember("SmallNumber") && cardFonts["SmallNumber"].IsObject()) {
            _smallNumberFont = parseFontInfoFromJson(cardFonts["SmallNumber"]);
        }
        
        if (cardFonts.HasMember("Suit") && cardFonts["Suit"].IsObject()) {
            _suitFont = parseFontInfoFromJson(cardFonts["Suit"]);
        }
        
        if (cardFonts.HasMember("CardBack") && cardFonts["CardBack"].IsObject()) {
            _cardBackFont = parseFontInfoFromJson(cardFonts["CardBack"]);
        }
    }
    
    // 解析UI字体配置
    if (json.HasMember("UIFonts") && json["UIFonts"].IsObject()) {
        const rapidjson::Value& uiFonts = json["UIFonts"];
        
        if (uiFonts.HasMember("Title") && uiFonts["Title"].IsObject()) {
            _titleFont = parseFontInfoFromJson(uiFonts["Title"]);
        }
        
        if (uiFonts.HasMember("Button") && uiFonts["Button"].IsObject()) {
            _buttonFont = parseFontInfoFromJson(uiFonts["Button"]);
        }
    }
    
    return isValid();
}

rapidjson::Value FontConfig::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value configJson(rapidjson::kObjectType);
    
    // 序列化卡牌字体配置
    rapidjson::Value cardFontsJson(rapidjson::kObjectType);
    cardFontsJson.AddMember("BigNumber", serializeFontInfoToJson(_bigNumberFont, allocator), allocator);
    cardFontsJson.AddMember("SmallNumber", serializeFontInfoToJson(_smallNumberFont, allocator), allocator);
    cardFontsJson.AddMember("Suit", serializeFontInfoToJson(_suitFont, allocator), allocator);
    cardFontsJson.AddMember("CardBack", serializeFontInfoToJson(_cardBackFont, allocator), allocator);
    configJson.AddMember("CardFonts", cardFontsJson, allocator);
    
    // 序列化UI字体配置
    rapidjson::Value uiFontsJson(rapidjson::kObjectType);
    uiFontsJson.AddMember("Title", serializeFontInfoToJson(_titleFont, allocator), allocator);
    uiFontsJson.AddMember("Button", serializeFontInfoToJson(_buttonFont, allocator), allocator);
    configJson.AddMember("UIFonts", uiFontsJson, allocator);
    
    return configJson;
}

bool FontConfig::isValid() const {
    return isValidFontInfo(_bigNumberFont) &&
           isValidFontInfo(_smallNumberFont) &&
           isValidFontInfo(_suitFont) &&
           isValidFontInfo(_cardBackFont) &&
           isValidFontInfo(_titleFont) &&
           isValidFontInfo(_buttonFont);
}

std::string FontConfig::getSummary() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Font - BigNum:%s/%.0f SmallNum:%s/%.0f Suit:%s/%.0f",
             _bigNumberFont.family.c_str(), _bigNumberFont.size,
             _smallNumberFont.family.c_str(), _smallNumberFont.size,
             _suitFont.family.c_str(), _suitFont.size);
    return std::string(buffer);
}

FontConfig::FontInfo FontConfig::parseFontInfoFromJson(const rapidjson::Value& json) const {
    FontInfo fontInfo;
    
    if (json.HasMember("family") && json["family"].IsString()) {
        fontInfo.family = json["family"].GetString();
    }
    
    if (json.HasMember("size") && json["size"].IsNumber()) {
        fontInfo.size = json["size"].GetFloat();
    }
    
    if (json.HasMember("text") && json["text"].IsString()) {
        fontInfo.text = json["text"].GetString();
    }
    
    return fontInfo;
}

rapidjson::Value FontConfig::serializeFontInfoToJson(const FontInfo& fontInfo, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value fontJson(rapidjson::kObjectType);
    
    rapidjson::Value familyValue(fontInfo.family.c_str(), allocator);
    fontJson.AddMember("family", familyValue, allocator);
    fontJson.AddMember("size", fontInfo.size, allocator);
    
    if (!fontInfo.text.empty()) {
        rapidjson::Value textValue(fontInfo.text.c_str(), allocator);
        fontJson.AddMember("text", textValue, allocator);
    }
    
    return fontJson;
}

bool FontConfig::isValidFontInfo(const FontInfo& fontInfo) const {
    if (fontInfo.family.empty()) {
        return false;
    }
    
    if (fontInfo.size <= 0 || fontInfo.size > 100.0f) {
        return false;
    }
    
    return true;
}
