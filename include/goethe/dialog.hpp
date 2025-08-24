#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <iosfwd>
#include <yaml-cpp/yaml.h>

// Export macro for shared library
#ifdef _WIN32
    #ifdef GOETHE_EXPORTS
        #define GOETHE_API __declspec(dllexport)
    #else
        #define GOETHE_API __declspec(dllimport)
    #endif
#else
    #define GOETHE_API __attribute__((visibility("default")))
#endif

// For convenience
using yaml = YAML::Node;

namespace goethe {

// Forward declarations
class DialogueRunner;
class IDialoguePort;

// Condition system (same grammar as Regent)
struct Condition {
    enum class Type {
        ALL, ANY, NOT,
        FLAG, VAR, QUEST_STATE, OBJECTIVE_STATE,
        CHAPTER_ACTIVE, AREA_ENTERED, DIALOGUE_VISITED,
        CHOICE_MADE, EVENT, TIME_SINCE, INVENTORY_HAS,
        DOOR_LOCKED, ACCESS_ALLOWED
    };
    
    Type type;
    std::string key;
    std::variant<std::string, int, float, bool> value;
    std::vector<Condition> children; // For ALL/ANY/NOT combinators
};

// Effect system (Regent effects)
struct Effect {
    enum class Type {
        SET_FLAG, SET_VAR, QUEST_ADD, QUEST_COMPLETE,
        NOTIFY, PLAY_SFX, PLAY_MUSIC, TELEPORT
    };
    
    Type type;
    std::string target;
    std::variant<std::string, int, float, bool> value;
    std::map<std::string, std::string> params;
};

// Voice/audio metadata
struct Voice {
    std::string clipId;
    bool subtitles = true;
    int startMs = 0;
};

// Portrait metadata
struct Portrait {
    std::string id;
    std::string mood;
};

// Line content (single line or weighted variant)
struct Line {
    std::string text; // i18n key
    std::optional<Voice> voice;
    std::optional<Portrait> portrait;
    std::vector<std::string> sfx;
    std::map<std::string, std::string> params; // i18n interpolation
    std::optional<Condition> conditions;
    float weight = 1.0f; // for weighted variants
};

// Choice definition
struct Choice {
    std::string id;
    std::string text; // i18n key
    std::string to; // nodeId or "$END"
    std::optional<Condition> conditions;
    std::vector<Effect> effects;
    bool once = false; // auto-hide after chosen
    int cooldownMs = 0; // resurfaces after time
    std::optional<std::string> disabledText; // i18n key for gated choices
};

// Node: one "beat" in the conversation
struct Node {
    std::string id;
    std::optional<std::string> speaker; // entity id
    std::vector<std::string> tags;
    
    // Line content (single or variants)
    std::optional<Line> line; // single line
    std::vector<Line> lines; // weighted variants
    
    std::vector<Choice> choices;
    
    // Effects
    std::vector<Effect> onEnterEffects;
    std::vector<Effect> onExitEffects;
    
    // Auto-advance
    std::optional<int> autoAdvanceMs; // if no choices
    
    bool interruptible = true;
};

// Dialogue: complete conversation structure
struct Dialogue {
    std::string id;
    std::map<std::string, std::string> metadata;
    std::vector<Node> nodes;
    std::optional<std::string> startNode;
    
    // Locals (dialogue scope)
    std::map<std::string, std::string> localVars;
};

// Runtime state
enum class DialogueState {
    IDLE,
    STARTING,
    RUNNING,
    WAITING_CHOICE,
    SUSPENDED,
    COMPLETED,
    ABORTED
};

// Snapshot for save/load
struct DialogueSnapshot {
    std::string dialogueId;
    std::string currentNodeId;
    std::map<std::string, std::string> localVars;
    int lineCursor = 0;
    int timeLeftMs = 0;
    std::vector<std::string> stack; // for sub-dialogs
};

// Renderer Port interface
class IDialoguePort {
public:
    virtual ~IDialoguePort() = default;
    
    struct Capabilities {
        bool supportsRichText = false;
        bool supportsPortraits = false;
        bool supportsDisabledChoices = false;
        bool supportsAutoAdvanceIndicator = false;
        bool supportsVoicePlayback = false;
    };
    
    struct LinePayload {
        std::string text;
        std::optional<Voice> voice;
        std::optional<Portrait> portrait;
        std::vector<std::string> sfx;
    };
    
    struct ChoicePayload {
        std::string id;
        std::string text;
        bool disabled = false;
    };
    
    struct NodePayload {
        std::string type; // "line", "choices", "meta"
        std::optional<LinePayload> line;
        std::optional<std::vector<ChoicePayload>> choices;
        std::optional<std::pair<std::string, std::string>> meta; // key, value
    };
    
    virtual Capabilities getCapabilities() = 0;
    virtual bool presentNode(const std::string& dialogueId, const std::string& nodeId, 
                           const std::vector<NodePayload>& payload) = 0;
};

// Events for EventBus
struct DialogueEvent {
    enum class Type {
        STARTED, SHOWN, CHOICE_OFFERED, CHOICE_SELECTED,
        SUSPENDED, RESUMED, COMPLETED, ABORTED
    };
    
    Type type;
    std::string dialogueId;
    std::string nodeId;
    std::optional<std::string> choiceId;
    std::optional<std::string> reason;
};

// Core functions
GOETHE_API Dialogue read_dialogue(std::istream& input);
GOETHE_API void write_dialogue(std::ostream& output, const Dialogue& dialogue);

// YAML conversion helpers
void from_yaml(const YAML::Node& node, Line& line);
YAML::Node to_yaml(const Line& line);
void from_yaml(const YAML::Node& node, Choice& choice);
YAML::Node to_yaml(const Choice& choice);
void from_yaml(const YAML::Node& node, Node& node_obj);
YAML::Node to_yaml(const Node& node_obj);
void from_yaml(const YAML::Node& node, Dialogue& dialogue);
YAML::Node to_yaml(const Dialogue& dialogue);

} // namespace goethe
