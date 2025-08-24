#!/bin/bash

# Create sample package for Goethe Dialog System
# This script creates sample dialog files in both legacy and GOETHE formats

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Creating sample package for Goethe Dialog System...${NC}"

# Create directories
mkdir -p sample_dialog_files
mkdir -p sample_dialog_files/legacy
mkdir -p sample_dialog_files/goethe

# Create legacy format sample (original format)
echo -e "${YELLOW}Creating legacy format sample...${NC}"

cat > sample_dialog_files/legacy/chapter1_intro.yaml << 'EOF'
dialogue_id: chapter1_intro
title: Chapter 1: The Beginning
mode: visual_novel
default_time: 3.0
lines:
  - character: Alice
    phrase: Hello, welcome to our story!
    direction: center
    expression: happy
    mood: friendly
    time: 2.5
  - character: Bob
    phrase: Thank you, I'm excited to begin!
    direction: left
    expression: excited
    mood: enthusiastic
    time: 3.0
  - character: Alice
    phrase: Let's start our adventure together!
    direction: center
    expression: happy
    mood: friendly
    time: 2.0
EOF

# Create GOETHE format sample (new format)
echo -e "${YELLOW}Creating GOETHE format sample...${NC}"

cat > sample_dialog_files/goethe/dlg_marshal.yaml << 'EOF'
kind: dialogue
id: dlg_marshal
startNode: intro

nodes:
  - id: intro
    speaker: marshal
    line:
      text: dlg_marshal.intro.text
      portrait: { id: marshal, mood: neutral }
      voice: { clipId: vo_marshal_intro }
    choices:
      - id: accept_help
        text: dlg_marshal.intro.choice.accept
        to: agree
        effects:
          - setFlag: accepted_president_help
          - quest.add: help_president
      - id: refuse_help
        text: dlg_marshal.intro.choice.refuse
        to: farewell

  - id: agree
    lines:
      - text: dlg_marshal.agree.v1
        weight: 2
      - text: dlg_marshal.agree.v2
        weight: 1
    autoAdvance: { ms: 500 }
    onExit:
      effects:
        - notify: { title: phrase_notify_title, body: phrase_quest_done, params: { title: quest.help_president.title } }
    choices:
      - id: proceed
        text: dlg_common.continue
        to: $END

  - id: farewell
    line:
      text: dlg_marshal.farewell
    choices:
      - id: close
        text: dlg_common.close
        to: $END
EOF

# Create a more complex GOETHE example with conditions
cat > sample_dialog_files/goethe/dlg_gate_example.yaml << 'EOF'
kind: dialogue
id: dlg_gate_example
startNode: gate_prompt

nodes:
  - id: gate_prompt
    speaker: guard
    line:
      text: dlg_gate.prompt
      portrait: { id: guard, mood: stern }
    choices:
      - id: locked_option
        text: dlg_gate.option.locked
        conditions: { not: { flag: accepted_president_help } }
        disabledText: dlg_gate.option.need_accept
        to: gate_prompt
      - id: unlocked_option
        text: dlg_gate.option.unlocked
        conditions: { flag: accepted_president_help }
        to: next_step

  - id: next_step
    line:
      text: dlg_gate.success
      voice: { clipId: vo_gate_success }
    autoAdvance: { ms: 2000 }
    onExit:
      effects:
        - setVar: { name: gate_unlocked, value: true }
    choices:
      - id: enter
        text: dlg_common.enter
        to: $END
EOF

# Create i18n locale file
echo -e "${YELLOW}Creating i18n locale file...${NC}"

cat > sample_dialog_files/goethe/locale_en-GB.yaml << 'EOF'
phrases:
  # Marshal dialogue
  dlg_marshal.intro.text: "We have an audience for you."
  dlg_marshal.intro.choice.accept: "I'm in."
  dlg_marshal.intro.choice.refuse: "I'd rather not get involved."
  dlg_marshal.agree.v1: "Good. Take this pass to the Presidency."
  dlg_marshal.agree.v2: "Excellent. Head to the Presidency immediately."
  dlg_marshal.farewell: "Very well. Dismissed."
  
  # Gate dialogue
  dlg_gate.prompt: "You cannot proceed yet."
  dlg_gate.option.locked: "Try to enter"
  dlg_gate.option.need_accept: "You need the Marshal's approval."
  dlg_gate.option.unlocked: "Enter the Presidency"
  dlg_gate.success: "The gate opens smoothly."
  
  # Common phrases
  dlg_common.continue: "Continue"
  dlg_common.close: "Close"
  dlg_common.enter: "Enter"
  
  # Notifications
  phrase_notify_title: "Quest Updated"
  phrase_quest_done: "Quest completed: {title}"
EOF

# Create a simple test script
echo -e "${YELLOW}Creating test script...${NC}"

cat > sample_dialog_files/test_goethe.cpp << 'EOF'
#include "goethe/dialog.hpp"
#include <iostream>
#include <fstream>

int main() {
    try {
        // Test loading GOETHE format
        std::ifstream file("dlg_marshal.yaml");
        if (!file.is_open()) {
            std::cerr << "Could not open dlg_marshal.yaml" << std::endl;
            return 1;
        }
        
        goethe::Dialogue dialogue = goethe::read_dialogue(file);
        
        std::cout << "Loaded dialogue: " << dialogue.id << std::endl;
        std::cout << "Number of nodes: " << dialogue.nodes.size() << std::endl;
        
        for (const auto& node : dialogue.nodes) {
            std::cout << "  Node: " << node.id;
            if (node.speaker) {
                std::cout << " (Speaker: " << *node.speaker << ")";
            }
            std::cout << std::endl;
            
            if (node.line) {
                std::cout << "    Line: " << node.line->text << std::endl;
            }
            
            if (!node.choices.empty()) {
                std::cout << "    Choices: " << node.choices.size() << std::endl;
                for (const auto& choice : node.choices) {
                    std::cout << "      - " << choice.id << ": " << choice.text << " -> " << choice.to << std::endl;
                }
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
EOF

# Create README for the sample package
echo -e "${YELLOW}Creating README...${NC}"

cat > sample_dialog_files/README.md << 'EOF'
# Goethe Dialog System - Sample Package

This package contains sample dialog files demonstrating both the legacy format and the new GOETHE format.

## File Structure

```
sample_dialog_files/
├── legacy/                    # Legacy format files
│   └── chapter1_intro.yaml   # Simple linear dialogue
├── goethe/                   # GOETHE format files
│   ├── dlg_marshal.yaml      # Basic branching dialogue
│   ├── dlg_gate_example.yaml # Dialogue with conditions
│   └── locale_en-GB.yaml     # i18n locale file
└── test_goethe.cpp           # Test program
```

## Legacy Format

The legacy format is a simple linear structure:

```yaml
dialogue_id: chapter1_intro
title: Chapter 1: The Beginning
mode: visual_novel
default_time: 3.0
lines:
  - character: Alice
    phrase: Hello, welcome to our story!
    direction: center
    expression: happy
    mood: friendly
    time: 2.5
```

## GOETHE Format

The GOETHE format supports complex branching dialogues with conditions, effects, and i18n:

```yaml
kind: dialogue
id: dlg_marshal
startNode: intro

nodes:
  - id: intro
    speaker: marshal
    line:
      text: dlg_marshal.intro.text  # i18n key
      portrait: { id: marshal, mood: neutral }
      voice: { clipId: vo_marshal_intro }
    choices:
      - id: accept_help
        text: dlg_marshal.intro.choice.accept
        to: agree
        effects:
          - setFlag: accepted_president_help
          - quest.add: help_president
```

## Key Features

### GOETHE Format Features:
- **Node-based structure** with branching
- **i18n support** with locale files
- **Conditions** for gating content
- **Effects** for game state changes
- **Voice and portrait** metadata
- **Auto-advance** timing
- **Choice system** with effects

### Legacy Format Features:
- **Simple linear** dialogue
- **Character expressions** and moods
- **Timing control** per line
- **Basic positioning**

## Testing

Compile and run the test program:

```bash
g++ -std=c++17 -I../../include test_goethe.cpp ../../src/engine/core/dialog.cpp -lyaml-cpp -o test_goethe
./test_goethe
```

## Migration

To migrate from legacy to GOETHE format:

1. Convert each `DialogueLine` to a `Node`
2. Replace direct text with i18n keys
3. Add choices for branching
4. Add conditions and effects as needed
5. Create locale files for text resolution
EOF

echo -e "${GREEN}Sample package created successfully!${NC}"
echo -e "${BLUE}Files created:${NC}"
echo -e "  ${GREEN}✓${NC} sample_dialog_files/legacy/chapter1_intro.yaml"
echo -e "  ${GREEN}✓${NC} sample_dialog_files/goethe/dlg_marshal.yaml"
echo -e "  ${GREEN}✓${NC} sample_dialog_files/goethe/dlg_gate_example.yaml"
echo -e "  ${GREEN}✓${NC} sample_dialog_files/goethe/locale_en-GB.yaml"
echo -e "  ${GREEN}✓${NC} sample_dialog_files/test_goethe.cpp"
echo -e "  ${GREEN}✓${NC} sample_dialog_files/README.md"
echo ""
echo -e "${YELLOW}To test the new GOETHE format:${NC}"
echo -e "  cd sample_dialog_files/goethe"
echo -e "  g++ -std=c++17 -I../../../include ../test_goethe.cpp ../../../src/engine/core/dialog.cpp -lyaml-cpp -o test_goethe"
echo -e "  ./test_goethe"
