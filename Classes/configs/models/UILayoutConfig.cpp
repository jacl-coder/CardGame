#include "UILayoutConfig.h"

UILayoutConfig::UILayoutConfig() {
    resetToDefault();
}

UILayoutConfig::~UILayoutConfig() {
}

void UILayoutConfig::resetToDefault() {
    // 默认位置配置（原硬编码值）
    _stackPosition = Vec2(100, 200);
    _currentCardPosition = Vec2(300, 200);
    _playfieldAreaOffset = Vec2(0, 300);
    
    // 默认间距配置
    _stackCardOffset = 30.0f;
    
    // 默认背景颜色配置
    _playfieldBgColor = ColorConfig(0.2f, 0.4f, 0.2f, 0.3f); // 淡绿色
    _stackBgColor = ColorConfig(0.4f, 0.2f, 0.2f, 0.3f);     // 淡红色
    
    // 默认背景尺寸配置
    _stackBgWidthRatio = 0.3f;
    _stackBgHeight = 200.0f;
}

bool UILayoutConfig::fromJson(const rapidjson::Value& json) {
    if (!json.IsObject()) {
        CCLOG("UILayoutConfig::fromJson - Invalid JSON format");
        return false;
    }
    
    // 解析位置配置
    if (json.HasMember("StackPosition") && json["StackPosition"].IsObject()) {
        _stackPosition = parseVec2FromJson(json["StackPosition"]);
    }
    
    if (json.HasMember("CurrentCardPosition") && json["CurrentCardPosition"].IsObject()) {
        _currentCardPosition = parseVec2FromJson(json["CurrentCardPosition"]);
    }
    
    if (json.HasMember("PlayfieldAreaOffset") && json["PlayfieldAreaOffset"].IsObject()) {
        _playfieldAreaOffset = parseVec2FromJson(json["PlayfieldAreaOffset"]);
    }
    
    // 解析间距配置
    if (json.HasMember("StackCardOffset") && json["StackCardOffset"].IsNumber()) {
        _stackCardOffset = json["StackCardOffset"].GetFloat();
    }
    
    // 解析背景颜色配置
    if (json.HasMember("BackgroundColors") && json["BackgroundColors"].IsObject()) {
        const rapidjson::Value& bgColors = json["BackgroundColors"];
        
        if (bgColors.HasMember("Playfield") && bgColors["Playfield"].IsObject()) {
            _playfieldBgColor = parseColorFromJson(bgColors["Playfield"]);
        }
        
        if (bgColors.HasMember("Stack") && bgColors["Stack"].IsObject()) {
            _stackBgColor = parseColorFromJson(bgColors["Stack"]);
        }
    }
    
    // 解析背景尺寸配置
    if (json.HasMember("StackBackgroundWidthRatio") && json["StackBackgroundWidthRatio"].IsNumber()) {
        _stackBgWidthRatio = json["StackBackgroundWidthRatio"].GetFloat();
    }
    
    if (json.HasMember("StackBackgroundHeight") && json["StackBackgroundHeight"].IsNumber()) {
        _stackBgHeight = json["StackBackgroundHeight"].GetFloat();
    }
    
    return isValid();
}

rapidjson::Value UILayoutConfig::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value configJson(rapidjson::kObjectType);
    
    // 序列化位置配置
    configJson.AddMember("StackPosition", serializeVec2ToJson(_stackPosition, allocator), allocator);
    configJson.AddMember("CurrentCardPosition", serializeVec2ToJson(_currentCardPosition, allocator), allocator);
    configJson.AddMember("PlayfieldAreaOffset", serializeVec2ToJson(_playfieldAreaOffset, allocator), allocator);
    
    // 序列化间距配置
    configJson.AddMember("StackCardOffset", _stackCardOffset, allocator);
    
    // 序列化背景颜色配置
    rapidjson::Value bgColorsJson(rapidjson::kObjectType);
    bgColorsJson.AddMember("Playfield", serializeColorToJson(_playfieldBgColor, allocator), allocator);
    bgColorsJson.AddMember("Stack", serializeColorToJson(_stackBgColor, allocator), allocator);
    configJson.AddMember("BackgroundColors", bgColorsJson, allocator);
    
    // 序列化背景尺寸配置
    configJson.AddMember("StackBackgroundWidthRatio", _stackBgWidthRatio, allocator);
    configJson.AddMember("StackBackgroundHeight", _stackBgHeight, allocator);
    
    return configJson;
}

bool UILayoutConfig::isValid() const {
    // 检查基本有效性
    if (_stackCardOffset < 0) {
        return false;
    }
    
    if (_stackBgWidthRatio <= 0 || _stackBgWidthRatio > 1.0f) {
        return false;
    }
    
    if (_stackBgHeight <= 0) {
        return false;
    }
    
    return true;
}

std::string UILayoutConfig::getSummary() const {
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
             "UILayout - Stack:(%.0f,%.0f) Current:(%.0f,%.0f) Offset:%.1f",
             _stackPosition.x, _stackPosition.y,
             _currentCardPosition.x, _currentCardPosition.y,
             _stackCardOffset);
    return std::string(buffer);
}

Vec2 UILayoutConfig::parseVec2FromJson(const rapidjson::Value& json) const {
    Vec2 result = Vec2::ZERO;
    
    if (json.HasMember("x") && json["x"].IsNumber()) {
        result.x = json["x"].GetFloat();
    }
    
    if (json.HasMember("y") && json["y"].IsNumber()) {
        result.y = json["y"].GetFloat();
    }
    
    return result;
}

rapidjson::Value UILayoutConfig::serializeVec2ToJson(const Vec2& vec, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value vecJson(rapidjson::kObjectType);
    vecJson.AddMember("x", vec.x, allocator);
    vecJson.AddMember("y", vec.y, allocator);
    return vecJson;
}

UILayoutConfig::ColorConfig UILayoutConfig::parseColorFromJson(const rapidjson::Value& json) const {
    ColorConfig color;
    
    if (json.HasMember("r") && json["r"].IsNumber()) {
        color.r = json["r"].GetFloat();
    }
    
    if (json.HasMember("g") && json["g"].IsNumber()) {
        color.g = json["g"].GetFloat();
    }
    
    if (json.HasMember("b") && json["b"].IsNumber()) {
        color.b = json["b"].GetFloat();
    }
    
    if (json.HasMember("a") && json["a"].IsNumber()) {
        color.a = json["a"].GetFloat();
    }
    
    return color;
}

rapidjson::Value UILayoutConfig::serializeColorToJson(const ColorConfig& color, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value colorJson(rapidjson::kObjectType);
    colorJson.AddMember("r", color.r, allocator);
    colorJson.AddMember("g", color.g, allocator);
    colorJson.AddMember("b", color.b, allocator);
    colorJson.AddMember("a", color.a, allocator);
    return colorJson;
}
