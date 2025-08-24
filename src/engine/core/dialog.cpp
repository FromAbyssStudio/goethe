#include "goethe/dialog.hpp"
#include "goethe/goethe_dialog.h"
#include <fstream>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <random>
#include <algorithm>

namespace goethe {

// ============================================================================
// YAML Conversion Helpers for GOETHE Structures
// ============================================================================

void from_yaml(const YAML::Node& node, Condition& condition) {
    if (node["all"]) {
        condition.type = Condition::Type::ALL;
        for (const auto& child : node["all"]) {
            Condition child_condition;
            from_yaml(child, child_condition);
            condition.children.push_back(child_condition);
        }
    } else if (node["any"]) {
        condition.type = Condition::Type::ANY;
        for (const auto& child : node["any"]) {
            Condition child_condition;
            from_yaml(child, child_condition);
            condition.children.push_back(child_condition);
        }
    } else if (node["not"]) {
        condition.type = Condition::Type::NOT;
        Condition child_condition;
        from_yaml(node["not"], child_condition);
        condition.children.push_back(child_condition);
    } else if (node["flag"]) {
        condition.type = Condition::Type::FLAG;
        condition.key = node["flag"].as<std::string>();
    } else if (node["var"]) {
        condition.type = Condition::Type::VAR;
        condition.key = node["var"]["name"].as<std::string>();
        if (node["var"]["value"].IsScalar()) {
            condition.value = node["var"]["value"].as<std::string>();
        }
    }
}

YAML::Node to_yaml(const Condition& condition) {
    YAML::Node node;
    switch (condition.type) {
        case Condition::Type::ALL: {
            YAML::Node children;
            for (const auto& child : condition.children) {
                children.push_back(to_yaml(child));
            }
            node["all"] = children;
            break;
        }
        case Condition::Type::ANY: {
            YAML::Node children;
            for (const auto& child : condition.children) {
                children.push_back(to_yaml(child));
            }
            node["any"] = children;
            break;
        }
        case Condition::Type::NOT: {
            if (!condition.children.empty()) {
                node["not"] = to_yaml(condition.children[0]);
            }
            break;
        }
        case Condition::Type::FLAG:
            node["flag"] = condition.key;
            break;
        case Condition::Type::VAR: {
            YAML::Node var_node;
            var_node["name"] = condition.key;
            std::visit([&var_node](const auto& v) {
                var_node["value"] = v;
            }, condition.value);
            node["var"] = var_node;
            break;
        }
        default:
            break;
    }
    return node;
}

void from_yaml(const YAML::Node& node, Effect& effect) {
    // Handle new format: type, target, value
    if (node["type"]) {
        std::string type_str = node["type"].as<std::string>();
        if (type_str == "SET_FLAG") {
            effect.type = Effect::Type::SET_FLAG;
        } else if (type_str == "SET_VAR") {
            effect.type = Effect::Type::SET_VAR;
        } else if (type_str == "QUEST_ADD") {
            effect.type = Effect::Type::QUEST_ADD;
        } else if (type_str == "QUEST_COMPLETE") {
            effect.type = Effect::Type::QUEST_COMPLETE;
        } else if (type_str == "NOTIFY") {
            effect.type = Effect::Type::NOTIFY;
        } else if (type_str == "PLAY_SFX") {
            effect.type = Effect::Type::PLAY_SFX;
        } else if (type_str == "PLAY_MUSIC") {
            effect.type = Effect::Type::PLAY_MUSIC;
        } else if (type_str == "TELEPORT") {
            effect.type = Effect::Type::TELEPORT;
        }
        
        if (node["target"]) {
            effect.target = node["target"].as<std::string>();
        }
        
        if (node["value"]) {
            if (node["value"].IsScalar()) {
                if (node["value"].IsSequence()) {
                    // Handle as string for now
                    effect.value = node["value"].as<std::string>();
                } else if (node["value"].IsMap()) {
                    // Handle as string for now
                    effect.value = node["value"].as<std::string>();
                } else {
                    // Try to determine the type
                    try {
                        effect.value = node["value"].as<bool>();
                    } catch (...) {
                        try {
                            effect.value = node["value"].as<int>();
                        } catch (...) {
                            try {
                                effect.value = node["value"].as<float>();
                            } catch (...) {
                                effect.value = node["value"].as<std::string>();
                            }
                        }
                    }
                }
            }
        }
        
        if (node["params"]) {
            for (const auto& param : node["params"]) {
                effect.params[param.first.as<std::string>()] = param.second.as<std::string>();
            }
        }
    }
    // Handle old format: setFlag, setVar, etc.
    else if (node["setFlag"]) {
        effect.type = Effect::Type::SET_FLAG;
        effect.target = node["setFlag"].as<std::string>();
    } else if (node["setVar"]) {
        effect.type = Effect::Type::SET_VAR;
        effect.target = node["setVar"]["name"].as<std::string>();
        if (node["setVar"]["value"].IsScalar()) {
            effect.value = node["setVar"]["value"].as<std::string>();
        }
    } else if (node["quest.add"]) {
        effect.type = Effect::Type::QUEST_ADD;
        effect.target = node["quest.add"].as<std::string>();
    } else if (node["notify"]) {
        effect.type = Effect::Type::NOTIFY;
        effect.target = node["notify"]["title"].as<std::string>();
        effect.value = node["notify"]["body"].as<std::string>();
    }
}

YAML::Node to_yaml(const Effect& effect) {
    YAML::Node node;
    switch (effect.type) {
        case Effect::Type::SET_FLAG:
            node["setFlag"] = effect.target;
            break;
        case Effect::Type::SET_VAR: {
            YAML::Node var_node;
            var_node["name"] = effect.target;
            std::visit([&var_node](const auto& v) {
                var_node["value"] = v;
            }, effect.value);
            node["setVar"] = var_node;
            break;
        }
        case Effect::Type::QUEST_ADD:
            node["quest.add"] = effect.target;
            break;
        case Effect::Type::NOTIFY: {
            YAML::Node notify_node;
            notify_node["title"] = effect.target;
            notify_node["body"] = std::get<std::string>(effect.value);
            node["notify"] = notify_node;
            break;
        }
        default:
            break;
    }
    return node;
}

void from_yaml(const YAML::Node& node, Voice& voice) {
    voice.clipId = node["clipId"].as<std::string>();
    voice.subtitles = node["subtitles"] ? node["subtitles"].as<bool>() : true;
    voice.startMs = node["startMs"] ? node["startMs"].as<int>() : 0;
}

YAML::Node to_yaml(const Voice& voice) {
    YAML::Node node;
    node["clipId"] = voice.clipId;
    if (!voice.subtitles) node["subtitles"] = voice.subtitles;
    if (voice.startMs > 0) node["startMs"] = voice.startMs;
    return node;
}

void from_yaml(const YAML::Node& node, Portrait& portrait) {
    portrait.id = node["id"].as<std::string>();
    portrait.mood = node["mood"] ? node["mood"].as<std::string>() : "";
}

YAML::Node to_yaml(const Portrait& portrait) {
    YAML::Node node;
    node["id"] = portrait.id;
    if (!portrait.mood.empty()) node["mood"] = portrait.mood;
    return node;
}

void from_yaml(const YAML::Node& node, Line& line) {
    line.text = node["text"].as<std::string>();
    
    if (node["voice"]) {
        Voice voice;
        from_yaml(node["voice"], voice);
        line.voice = voice;
    }
    
    if (node["portrait"]) {
        Portrait portrait;
        from_yaml(node["portrait"], portrait);
        line.portrait = portrait;
    }
    
    if (node["sfx"]) {
        for (const auto& sfx : node["sfx"]) {
            line.sfx.push_back(sfx.as<std::string>());
        }
    }
    
    if (node["params"]) {
        for (const auto& param : node["params"]) {
            line.params[param.first.as<std::string>()] = param.second.as<std::string>();
        }
    }
    
    if (node["conditions"]) {
        Condition condition;
        from_yaml(node["conditions"], condition);
        line.conditions = condition;
    }
    
    line.weight = node["weight"] ? node["weight"].as<float>() : 1.0f;
}

YAML::Node to_yaml(const Line& line) {
    YAML::Node node;
    node["text"] = line.text;
    
    if (line.voice) {
        node["voice"] = to_yaml(*line.voice);
    }
    
    if (line.portrait) {
        node["portrait"] = to_yaml(*line.portrait);
    }
    
    if (!line.sfx.empty()) {
        YAML::Node sfx_node;
        for (const auto& sfx : line.sfx) {
            sfx_node.push_back(sfx);
        }
        node["sfx"] = sfx_node;
    }
    
    if (!line.params.empty()) {
        YAML::Node params_node;
        for (const auto& param : line.params) {
            params_node[param.first] = param.second;
        }
        node["params"] = params_node;
    }
    
    if (line.conditions) {
        node["conditions"] = to_yaml(*line.conditions);
    }
    
    if (line.weight != 1.0f) {
        node["weight"] = line.weight;
    }
    
    return node;
}

void from_yaml(const YAML::Node& node, Choice& choice) {
    choice.id = node["id"].as<std::string>();
    choice.text = node["text"].as<std::string>();
    choice.to = node["to"].as<std::string>();
    
    if (node["conditions"]) {
        Condition condition;
        from_yaml(node["conditions"], condition);
        choice.conditions = condition;
    }
    
    if (node["effects"]) {
        for (const auto& effect_node : node["effects"]) {
            Effect effect;
            from_yaml(effect_node, effect);
            choice.effects.push_back(effect);
        }
    }
    
    choice.once = node["once"] ? node["once"].as<bool>() : false;
    choice.cooldownMs = node["cooldownMs"] ? node["cooldownMs"].as<int>() : 0;
    
    if (node["disabledText"]) {
        choice.disabledText = node["disabledText"].as<std::string>();
    }
}

YAML::Node to_yaml(const Choice& choice) {
    YAML::Node node;
    node["id"] = choice.id;
    node["text"] = choice.text;
    node["to"] = choice.to;
    
    if (choice.conditions) {
        node["conditions"] = to_yaml(*choice.conditions);
    }
    
    if (!choice.effects.empty()) {
        YAML::Node effects_node;
        for (const auto& effect : choice.effects) {
            effects_node.push_back(to_yaml(effect));
        }
        node["effects"] = effects_node;
    }
    
    if (choice.once) node["once"] = choice.once;
    if (choice.cooldownMs > 0) node["cooldownMs"] = choice.cooldownMs;
    if (choice.disabledText) node["disabledText"] = *choice.disabledText;
    
    return node;
}

void from_yaml(const YAML::Node& node, Node& node_obj) {
    node_obj.id = node["id"].as<std::string>();
    node_obj.speaker = node["speaker"] ? node["speaker"].as<std::string>() : std::optional<std::string>();
    
    if (node["tags"]) {
        for (const auto& tag : node["tags"]) {
            node_obj.tags.push_back(tag.as<std::string>());
        }
    }
    
    if (node["line"]) {
        Line line;
        from_yaml(node["line"], line);
        node_obj.line = line;
    } else if (node["lines"]) {
        for (const auto& line_node : node["lines"]) {
            Line line;
            from_yaml(line_node, line);
            node_obj.lines.push_back(line);
        }
    }
    
    if (node["choices"]) {
        for (const auto& choice_node : node["choices"]) {
            Choice choice;
            from_yaml(choice_node, choice);
            node_obj.choices.push_back(choice);
        }
    }
    
    if (node["onEnter"] && node["onEnter"]["effects"]) {
        for (const auto& effect_node : node["onEnter"]["effects"]) {
            Effect effect;
            from_yaml(effect_node, effect);
            node_obj.onEnterEffects.push_back(effect);
        }
    }
    
    if (node["onExit"] && node["onExit"]["effects"]) {
        for (const auto& effect_node : node["onExit"]["effects"]) {
            Effect effect;
            from_yaml(effect_node, effect);
            node_obj.onExitEffects.push_back(effect);
        }
    }
    
    // Handle new format: autoAdvanceMs
    if (node["autoAdvanceMs"]) {
        node_obj.autoAdvanceMs = node["autoAdvanceMs"].as<int>();
    }
    // Handle old format: autoAdvance.ms
    else if (node["autoAdvance"]) {
        node_obj.autoAdvanceMs = node["autoAdvance"]["ms"].as<int>();
    }
    
    node_obj.interruptible = node["interruptible"] ? node["interruptible"].as<bool>() : true;
}

YAML::Node to_yaml(const Node& node_obj) {
    YAML::Node node;
    node["id"] = node_obj.id;
    
    if (node_obj.speaker) {
        node["speaker"] = *node_obj.speaker;
    }
    
    if (!node_obj.tags.empty()) {
        YAML::Node tags_node;
        for (const auto& tag : node_obj.tags) {
            tags_node.push_back(tag);
        }
        node["tags"] = tags_node;
    }
    
    if (node_obj.line) {
        node["line"] = to_yaml(*node_obj.line);
    } else if (!node_obj.lines.empty()) {
        YAML::Node lines_node;
        for (const auto& line : node_obj.lines) {
            lines_node.push_back(to_yaml(line));
        }
        node["lines"] = lines_node;
    }
    
    if (!node_obj.choices.empty()) {
        YAML::Node choices_node;
        for (const auto& choice : node_obj.choices) {
            choices_node.push_back(to_yaml(choice));
        }
        node["choices"] = choices_node;
    }
    
    if (!node_obj.onEnterEffects.empty()) {
        YAML::Node on_enter_node;
        YAML::Node effects_node;
        for (const auto& effect : node_obj.onEnterEffects) {
            effects_node.push_back(to_yaml(effect));
        }
        on_enter_node["effects"] = effects_node;
        node["onEnter"] = on_enter_node;
    }
    
    if (!node_obj.onExitEffects.empty()) {
        YAML::Node on_exit_node;
        YAML::Node effects_node;
        for (const auto& effect : node_obj.onExitEffects) {
            effects_node.push_back(to_yaml(effect));
        }
        on_exit_node["effects"] = effects_node;
        node["onExit"] = on_exit_node;
    }
    
    if (node_obj.autoAdvanceMs) {
        node["autoAdvanceMs"] = *node_obj.autoAdvanceMs;
    }
    
    if (!node_obj.interruptible) {
        node["interruptible"] = node_obj.interruptible;
    }
    
    return node;
}

void from_yaml(const YAML::Node& node, Dialogue& dialogue) {
    if (!node["id"]) {
        throw std::runtime_error("Dialogue missing required 'id' field");
    }
    dialogue.id = node["id"].as<std::string>();
    
    if (node["metadata"]) {
        for (const auto& meta : node["metadata"]) {
            dialogue.metadata[meta.first.as<std::string>()] = meta.second.as<std::string>();
        }
    }
    
    if (node["startNode"]) {
        dialogue.startNode = node["startNode"].as<std::string>();
    }
    
    if (!node["nodes"]) {
        throw std::runtime_error("Dialogue missing required 'nodes' field");
    }
    
    dialogue.nodes.clear();
    for (const auto& node_node : node["nodes"]) {
        Node node_obj;
        from_yaml(node_node, node_obj);
        dialogue.nodes.push_back(node_obj);
    }
    
    if (node["localVars"]) {
        for (const auto& var : node["localVars"]) {
            dialogue.localVars[var.first.as<std::string>()] = var.second.as<std::string>();
        }
    }
}

YAML::Node to_yaml(const Dialogue& dialogue) {
    YAML::Node node;
    node["kind"] = "dialogue";
    node["id"] = dialogue.id;
    
    if (!dialogue.metadata.empty()) {
        YAML::Node metadata_node;
        for (const auto& meta : dialogue.metadata) {
            metadata_node[meta.first] = meta.second;
        }
        node["metadata"] = metadata_node;
    }
    
    if (dialogue.startNode) {
        node["startNode"] = *dialogue.startNode;
    }
    
    YAML::Node nodes_node;
    for (const auto& node_obj : dialogue.nodes) {
        nodes_node.push_back(to_yaml(node_obj));
    }
    node["nodes"] = nodes_node;
    
    if (!dialogue.localVars.empty()) {
        YAML::Node local_vars_node;
        for (const auto& var : dialogue.localVars) {
            local_vars_node[var.first] = var.second;
        }
        node["localVars"] = local_vars_node;
    }
    
    return node;
}

// ============================================================================
// Core Functions
// ============================================================================

goethe::Dialogue read_dialogue(std::istream& input) {
    try {
        YAML::Node node = YAML::Load(input);
        if (!node.IsMap()) {
            throw std::runtime_error("Invalid dialogue format: root must be a map");
        }
        
        goethe::Dialogue dialogue;
        from_yaml(node, dialogue);
        return dialogue;
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML parsing error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw; // Re-throw existing exceptions
    } catch (...) {
        throw std::runtime_error("Unknown error while parsing dialogue");
    }
}

void write_dialogue(std::ostream& output, const goethe::Dialogue& dialogue) {
    YAML::Node node = to_yaml(dialogue);
    output << node;
}

} // namespace goethe

// ============================================================================
// C API Implementation (Updated for GOETHE structures)
// ============================================================================

extern "C" {

// Internal dialog structure for C API
struct DialogImpl {
    goethe::Dialogue dialogue;
    std::vector<std::string> string_storage; // Keep strings alive
};

GOETHE_API GoetheDialog* goethe_dialog_create(void) {
    return reinterpret_cast<GoetheDialog*>(new DialogImpl());
}

GOETHE_API void goethe_dialog_destroy(GoetheDialog* dialog) {
    if (dialog) {
        delete reinterpret_cast<DialogImpl*>(dialog);
    }
}

GOETHE_API int goethe_dialog_load_from_file(GoetheDialog* dialog, const char* filename) {
    if (!dialog || !filename) return -1;
    
    try {
        std::ifstream file(filename);
        if (!file.is_open()) return -1;
        
        auto impl = reinterpret_cast<DialogImpl*>(dialog);
        impl->dialogue = goethe::read_dialogue(file);
        return 0;
    } catch (...) {
        return -1;
    }
}

GOETHE_API int goethe_dialog_save_to_file(const GoetheDialog* dialog, const char* filename) {
    if (!dialog || !filename) return -1;
    
    try {
        std::ofstream file(filename);
        if (!file.is_open()) return -1;
        
        auto impl = reinterpret_cast<const DialogImpl*>(dialog);
        goethe::write_dialogue(file, impl->dialogue);
        return 0;
    } catch (...) {
        return -1;
    }
}

} // extern "C"
