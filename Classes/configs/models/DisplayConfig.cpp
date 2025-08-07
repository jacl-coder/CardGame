#include "DisplayConfig.h"

DisplayConfig::DisplayConfig() {
    resetToDefault();
}

DisplayConfig::~DisplayConfig() {
}

void DisplayConfig::resetToDefault() {
    // 默认设计分辨率配置（原硬编码值）
    _designResolution = ResolutionInfo("design", 1080, 2080);
    
    // 默认窗口配置
    _windowScale = 0.5f;
    _resolutionPolicy = "FIXED_WIDTH";
    _windowTitle = "CardGame";
    
    // 默认支持的分辨率列表（原硬编码值）
    _supportedResolutions = {
        ResolutionInfo("small", 480, 320),
        ResolutionInfo("medium", 1024, 768),
        ResolutionInfo("large", 2048, 1536),
        ResolutionInfo("design", 1080, 2080)
    };
}

bool DisplayConfig::fromJson(const rapidjson::Value& json) {
    if (!json.IsObject()) {
        CCLOG("DisplayConfig::fromJson - Invalid JSON format");
        return false;
    }
    
    // 解析显示配置
    if (json.HasMember("Display") && json["Display"].IsObject()) {
        const rapidjson::Value& display = json["Display"];
        
        // 解析设计分辨率
        if (display.HasMember("DesignResolution") && display["DesignResolution"].IsObject()) {
            _designResolution = parseResolutionInfoFromJson(display["DesignResolution"]);
        }
        
        // 解析窗口配置
        if (display.HasMember("WindowScale") && display["WindowScale"].IsNumber()) {
            _windowScale = display["WindowScale"].GetFloat();
        }
        
        if (display.HasMember("ResolutionPolicy") && display["ResolutionPolicy"].IsString()) {
            _resolutionPolicy = display["ResolutionPolicy"].GetString();
        }
        
        if (display.HasMember("WindowTitle") && display["WindowTitle"].IsString()) {
            _windowTitle = display["WindowTitle"].GetString();
        }
        
        // 解析支持的分辨率列表
        if (display.HasMember("SupportedResolutions") && display["SupportedResolutions"].IsArray()) {
            const rapidjson::Value& resolutions = display["SupportedResolutions"];
            _supportedResolutions.clear();
            
            for (rapidjson::SizeType i = 0; i < resolutions.Size(); i++) {
                if (resolutions[i].IsObject()) {
                    _supportedResolutions.push_back(parseResolutionInfoFromJson(resolutions[i]));
                }
            }
        }
    }
    
    return isValid();
}

rapidjson::Value DisplayConfig::toJson(rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value configJson(rapidjson::kObjectType);
    
    // 序列化显示配置
    rapidjson::Value displayJson(rapidjson::kObjectType);
    
    // 序列化设计分辨率
    displayJson.AddMember("DesignResolution", serializeResolutionInfoToJson(_designResolution, allocator), allocator);
    
    // 序列化窗口配置
    displayJson.AddMember("WindowScale", _windowScale, allocator);
    
    rapidjson::Value policyValue(_resolutionPolicy.c_str(), allocator);
    displayJson.AddMember("ResolutionPolicy", policyValue, allocator);
    
    rapidjson::Value titleValue(_windowTitle.c_str(), allocator);
    displayJson.AddMember("WindowTitle", titleValue, allocator);
    
    // 序列化支持的分辨率列表
    rapidjson::Value resolutionsArray(rapidjson::kArrayType);
    for (const auto& resolution : _supportedResolutions) {
        resolutionsArray.PushBack(serializeResolutionInfoToJson(resolution, allocator), allocator);
    }
    displayJson.AddMember("SupportedResolutions", resolutionsArray, allocator);
    
    configJson.AddMember("Display", displayJson, allocator);
    
    return configJson;
}

bool DisplayConfig::isValid() const {
    // 检查设计分辨率有效性
    if (!isValidResolutionInfo(_designResolution)) {
        return false;
    }
    
    // 检查窗口缩放有效性
    if (_windowScale <= 0.0f || _windowScale > 2.0f) {
        return false;
    }
    
    // 检查分辨率策略有效性
    if (_resolutionPolicy.empty()) {
        return false;
    }
    
    // 检查支持的分辨率列表
    for (const auto& resolution : _supportedResolutions) {
        if (!isValidResolutionInfo(resolution)) {
            return false;
        }
    }
    
    return true;
}

std::string DisplayConfig::getSummary() const {
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "Display - Design:%dx%d Scale:%.1f Policy:%s Resolutions:%zu",
             _designResolution.width, _designResolution.height,
             _windowScale, _resolutionPolicy.c_str(),
             _supportedResolutions.size());
    return std::string(buffer);
}

ResolutionPolicy DisplayConfig::getResolutionPolicyType() const {
    return parseResolutionPolicy(_resolutionPolicy);
}

DisplayConfig::ResolutionInfo DisplayConfig::getResolutionByName(const std::string& name) const {
    for (const auto& resolution : _supportedResolutions) {
        if (resolution.name == name) {
            return resolution;
        }
    }
    return _designResolution; // 默认返回设计分辨率
}

DisplayConfig::ResolutionInfo DisplayConfig::parseResolutionInfoFromJson(const rapidjson::Value& json) const {
    ResolutionInfo resolution;
    
    if (json.HasMember("name") && json["name"].IsString()) {
        resolution.name = json["name"].GetString();
    }
    
    if (json.HasMember("width") && json["width"].IsInt()) {
        resolution.width = json["width"].GetInt();
    }
    
    if (json.HasMember("height") && json["height"].IsInt()) {
        resolution.height = json["height"].GetInt();
    }
    
    return resolution;
}

rapidjson::Value DisplayConfig::serializeResolutionInfoToJson(const ResolutionInfo& resolution, rapidjson::Document::AllocatorType& allocator) const {
    rapidjson::Value resolutionJson(rapidjson::kObjectType);
    
    rapidjson::Value nameValue(resolution.name.c_str(), allocator);
    resolutionJson.AddMember("name", nameValue, allocator);
    resolutionJson.AddMember("width", resolution.width, allocator);
    resolutionJson.AddMember("height", resolution.height, allocator);
    
    return resolutionJson;
}

bool DisplayConfig::isValidResolutionInfo(const ResolutionInfo& resolution) const {
    if (resolution.name.empty()) {
        return false;
    }
    
    if (resolution.width <= 0 || resolution.width > 10000) {
        return false;
    }
    
    if (resolution.height <= 0 || resolution.height > 10000) {
        return false;
    }
    
    return true;
}

ResolutionPolicy DisplayConfig::parseResolutionPolicy(const std::string& policy) const {
    if (policy == "EXACT_FIT") return ResolutionPolicy::EXACT_FIT;
    if (policy == "NO_BORDER") return ResolutionPolicy::NO_BORDER;
    if (policy == "SHOW_ALL") return ResolutionPolicy::SHOW_ALL;
    if (policy == "FIXED_HEIGHT") return ResolutionPolicy::FIXED_HEIGHT;
    if (policy == "FIXED_WIDTH") return ResolutionPolicy::FIXED_WIDTH;

    return ResolutionPolicy::FIXED_WIDTH; // 默认策略
}
