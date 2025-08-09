#include "LevelConfigLoader.h"
#include "external/json/document.h"
#include "external/json/writer.h"
#include "external/json/stringbuffer.h"

LevelConfigLoader::LevelConfigLoader() {
}

LevelConfigLoader::~LevelConfigLoader() {
    clearCache();
}

std::shared_ptr<LevelConfig> LevelConfigLoader::loadLevelConfig(int levelId) {
    // 先检查缓存
    auto it = _cachedConfigs.find(levelId);
    if (it != _cachedConfigs.end()) {
    // found cached config
        return it->second;
    }
    
    // 从文件加载
    std::string filePath = getLevelConfigFilePath(levelId);
    auto config = loadLevelConfigFromFile(filePath);
    
    if (config) {
        // 设置关卡ID并缓存
        config->setLevelId(levelId);
        _cachedConfigs[levelId] = config;
        CCLOG("LevelConfigLoader::loadLevelConfig - Loaded and cached level %d", levelId);
    }
    
    return config;
}

std::shared_ptr<LevelConfig> LevelConfigLoader::loadLevelConfigFromFile(const std::string& filePath) {
    // loading from file
    
    std::string content = readFileContent(filePath);
    if (content.empty()) {
        CCLOG("LevelConfigLoader::loadLevelConfigFromFile - Failed to read file: %s", filePath.c_str());
        return nullptr;
    }
    
    return loadLevelConfigFromString(content);
}

std::shared_ptr<LevelConfig> LevelConfigLoader::loadLevelConfigFromString(const std::string& jsonString, int levelId) {
    rapidjson::Document document;
    if (!parseJsonDocument(jsonString, document)) {
        CCLOG("LevelConfigLoader::loadLevelConfigFromString - Failed to parse JSON");
        return nullptr;
    }
    
    if (!validateJsonDocument(document)) {
        CCLOG("LevelConfigLoader::loadLevelConfigFromString - Invalid JSON format");
        return nullptr;
    }
    
    auto config = std::make_shared<LevelConfig>();

    // 如果提供了关卡ID，先设置它（在fromJson之前）
    if (levelId > 0) {
        config->setLevelId(levelId);
    }

    if (!config->fromJson(document)) {
        CCLOG("LevelConfigLoader::loadLevelConfigFromString - Failed to create config from JSON");
        return nullptr;
    }
    
    // loaded config
    
    return config;
}

int LevelConfigLoader::preloadAllLevelConfigs(const std::string& configDirectory) {
    // preload from directory
    
    int loadedCount = 0;
    
    // 尝试加载关卡1-10（可以根据需要调整范围）
    for (int levelId = 1; levelId <= 10; levelId++) {
        auto config = loadLevelConfig(levelId);
        if (config) {
            loadedCount++;
        }
    }
    
    // loaded count
    return loadedCount;
}

std::shared_ptr<LevelConfig> LevelConfigLoader::getCachedLevelConfig(int levelId) const {
    auto it = _cachedConfigs.find(levelId);
    return (it != _cachedConfigs.end()) ? it->second : nullptr;
}

void LevelConfigLoader::clearCache() {
    _cachedConfigs.clear();
    // cache cleared
}

int LevelConfigLoader::getLoadedLevelCount() const {
    return static_cast<int>(_cachedConfigs.size());
}

std::vector<int> LevelConfigLoader::getLoadedLevelIds() const {
    std::vector<int> levelIds;
    for (const auto& pair : _cachedConfigs) {
        levelIds.push_back(pair.first);
    }
    return levelIds;
}

bool LevelConfigLoader::validateConfigFile(const std::string& filePath) const {
    std::string content = readFileContent(filePath);
    if (content.empty()) {
        return false;
    }
    
    rapidjson::Document document;
    if (!parseJsonDocument(content, document)) {
        return false;
    }
    
    return validateJsonDocument(document);
}



bool LevelConfigLoader::saveLevelConfig(std::shared_ptr<LevelConfig> levelConfig, const std::string& filePath) const {
    if (!levelConfig) {
        CCLOG("LevelConfigLoader::saveLevelConfig - Invalid config");
        return false;
    }

    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();

    rapidjson::Value configJson = levelConfig->toJson(allocator);
    document.CopyFrom(configJson, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    std::string jsonString = buffer.GetString();

    // 写入文件（这里简化处理，实际项目中可能需要更复杂的文件操作）
    FILE* file = fopen(filePath.c_str(), "w");
    if (!file) {
        CCLOG("LevelConfigLoader::saveLevelConfig - Failed to open file for writing: %s", filePath.c_str());
        return false;
    }

    size_t written = fwrite(jsonString.c_str(), 1, jsonString.length(), file);
    fclose(file);

    bool success = (written == jsonString.length());
    CCLOG("LevelConfigLoader::saveLevelConfig - %s: %s",
          success ? "Success" : "Failed", filePath.c_str());

    return success;
}

bool LevelConfigLoader::parseJsonDocument(const std::string& jsonString, rapidjson::Document& document) const {
    document.Parse(jsonString.c_str());

    if (document.HasParseError()) {
        CCLOG("LevelConfigLoader::parseJsonDocument - Parse error at offset %zu: %d",
              document.GetErrorOffset(), document.GetParseError());
        return false;
    }

    return true;
}

std::string LevelConfigLoader::readFileContent(const std::string& filePath) const {
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filePath);

    if (fullPath.empty()) {
        CCLOG("LevelConfigLoader::readFileContent - File not found: %s", filePath.c_str());
        return "";
    }

    Data data = FileUtils::getInstance()->getDataFromFile(fullPath);
    if (data.isNull()) {
        CCLOG("LevelConfigLoader::readFileContent - Failed to read file: %s", fullPath.c_str());
        return "";
    }

    return std::string(reinterpret_cast<const char*>(data.getBytes()), data.getSize());
}

std::string LevelConfigLoader::getLevelConfigFilePath(int levelId) const {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "configs/data/levels/level_%d.json", levelId);
    return std::string(buffer);
}

bool LevelConfigLoader::validateJsonDocument(const rapidjson::Document& document) const {
    if (!document.IsObject()) {
        CCLOG("LevelConfigLoader::validateJsonDocument - Root is not an object");
        return false;
    }

    // 检查必需的字段
    if (!document.HasMember("Playfield") || !document["Playfield"].IsArray()) {
        CCLOG("LevelConfigLoader::validateJsonDocument - Missing or invalid Playfield array");
        return false;
    }

    if (!document.HasMember("Stack") || !document["Stack"].IsArray()) {
        CCLOG("LevelConfigLoader::validateJsonDocument - Missing or invalid Stack array");
        return false;
    }

    // 检查Playfield数组中的卡牌格式
    const rapidjson::Value& playfield = document["Playfield"];
    for (rapidjson::SizeType i = 0; i < playfield.Size(); i++) {
        const rapidjson::Value& card = playfield[i];
        if (!card.IsObject() ||
            !card.HasMember("CardFace") || !card["CardFace"].IsInt() ||
            !card.HasMember("CardSuit") || !card["CardSuit"].IsInt() ||
            !card.HasMember("Position") || !card["Position"].IsObject()) {
            CCLOG("LevelConfigLoader::validateJsonDocument - Invalid Playfield card at index %u", i);
            return false;
        }
    }

    // 检查Stack数组中的卡牌格式
    const rapidjson::Value& stack = document["Stack"];
    for (rapidjson::SizeType i = 0; i < stack.Size(); i++) {
        const rapidjson::Value& card = stack[i];
        if (!card.IsObject() ||
            !card.HasMember("CardFace") || !card["CardFace"].IsInt() ||
            !card.HasMember("CardSuit") || !card["CardSuit"].IsInt()) {
            CCLOG("LevelConfigLoader::validateJsonDocument - Invalid Stack card at index %u", i);
            return false;
        }
    }

    return true;
}
