#include "AnimationConfig.h"

AnimationConfig::AnimationConfig() {
    resetToDefault();
}

AnimationConfig::~AnimationConfig() {
}

void AnimationConfig::resetToDefault() {
    // 默认动画时长配置（原硬编码值）
    _moveAnimationDuration = 0.3f;
    _flipAnimationDuration = 0.2f;
    _scaleAnimationDuration = 0.15f;
    _highlightAnimationDuration = 0.1f;
    
    // 默认动画效果配置
    _highlightScaleFactor = 1.1f;
    _clickScaleFactor = 1.2f;
}

bool AnimationConfig::fromJson(const rapidjson::Value& json) {
    if (!json.IsObject()) {
        CCLOG("AnimationConfig::fromJson - Invalid JSON format");
        return false;
    }
    
    // 解析动画时长配置
    if (json.HasMember("MoveAnimationDuration") && json["MoveAnimationDuration"].IsNumber()) {
        _moveAnimationDuration = json["MoveAnimationDuration"].GetFloat();
    }
    
    if (json.HasMember("FlipAnimationDuration") && json["FlipAnimationDuration"].IsNumber()) {
        _flipAnimationDuration = json["FlipAnimationDuration"].GetFloat();
    }
    
    if (json.HasMember("ScaleAnimationDuration") && json["ScaleAnimationDuration"].IsNumber()) {
        _scaleAnimationDuration = json["ScaleAnimationDuration"].GetFloat();
    }
    
    if (json.HasMember("HighlightAnimationDuration") && json["HighlightAnimationDuration"].IsNumber()) {
        _highlightAnimationDuration = json["HighlightAnimationDuration"].GetFloat();
    }
    
    // 解析动画效果配置
    if (json.HasMember("HighlightScaleFactor") && json["HighlightScaleFactor"].IsNumber()) {
        _highlightScaleFactor = json["HighlightScaleFactor"].GetFloat();
    }
    
    if (json.HasMember("ClickScaleFactor") && json["ClickScaleFactor"].IsNumber()) {
        _clickScaleFactor = json["ClickScaleFactor"].GetFloat();
    }
    
    return isValid();
}

rapidjson::Value AnimationConfig::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value configJson(rapidjson::kObjectType);
    
    // 序列化动画时长配置
    configJson.AddMember("MoveAnimationDuration", _moveAnimationDuration, allocator);
    configJson.AddMember("FlipAnimationDuration", _flipAnimationDuration, allocator);
    configJson.AddMember("ScaleAnimationDuration", _scaleAnimationDuration, allocator);
    configJson.AddMember("HighlightAnimationDuration", _highlightAnimationDuration, allocator);
    
    // 序列化动画效果配置
    configJson.AddMember("HighlightScaleFactor", _highlightScaleFactor, allocator);
    configJson.AddMember("ClickScaleFactor", _clickScaleFactor, allocator);
    
    return configJson;
}

bool AnimationConfig::isValid() const {
    // 检查动画时长有效性
    if (_moveAnimationDuration <= 0 || _moveAnimationDuration > 5.0f) {
        return false;
    }
    
    if (_flipAnimationDuration <= 0 || _flipAnimationDuration > 5.0f) {
        return false;
    }
    
    if (_scaleAnimationDuration <= 0 || _scaleAnimationDuration > 5.0f) {
        return false;
    }
    
    if (_highlightAnimationDuration <= 0 || _highlightAnimationDuration > 5.0f) {
        return false;
    }
    
    // 检查缩放因子有效性
    if (_highlightScaleFactor <= 0 || _highlightScaleFactor > 3.0f) {
        return false;
    }
    
    if (_clickScaleFactor <= 0 || _clickScaleFactor > 3.0f) {
        return false;
    }
    
    return true;
}

std::string AnimationConfig::getSummary() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Animation - Move:%.2fs Flip:%.2fs Scale:%.2fs Highlight:%.2fs",
             _moveAnimationDuration, _flipAnimationDuration,
             _scaleAnimationDuration, _highlightAnimationDuration);
    return std::string(buffer);
}
