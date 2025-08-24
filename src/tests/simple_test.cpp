#include "goethe/dialog.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

int main() {
    std::cout << "Goethe Dialog System Test" << std::endl;
    std::cout << "=========================" << std::endl;

    // Test 1: Simple format
    std::cout << "\n1. Testing Simple Format:" << std::endl;
    std::string simple_yaml = R"(
id: test_simple
nodes:
  - id: greeting
    speaker: alice
    line:
      text: Hello from simple format!
  - id: response
    speaker: bob
    line:
      text: This is a simple dialogue.
)";

    try {
        std::istringstream simple_stream(simple_yaml);
        goethe::Dialogue simple_dialogue = goethe::read_dialogue(simple_stream);
        
        std::cout << "  ✓ Loaded simple dialogue: " << simple_dialogue.id << std::endl;
        std::cout << "  ✓ Nodes: " << simple_dialogue.nodes.size() << std::endl;
        
        for (const auto& node : simple_dialogue.nodes) {
            std::cout << "    Node: " << node.id;
            if (node.speaker) {
                std::cout << " (Speaker: " << *node.speaker << ")";
            }
            std::cout << std::endl;
            
            if (node.line) {
                std::cout << "      Line: " << node.line->text << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "  ✗ Error loading simple format: " << e.what() << std::endl;
        return 1;
    }

    // Test 2: GOETHE format
    std::cout << "\n2. Testing GOETHE Format:" << std::endl;
    std::string goethe_yaml = R"(
kind: dialogue
id: test_goethe
startNode: intro

nodes:
  - id: intro
    speaker: marshal
    line:
      text: dlg_test.intro.text
      portrait: { id: marshal, mood: neutral }
      voice: { clipId: vo_test_intro }
    choices:
      - id: accept
        text: dlg_test.intro.choice.accept
        to: agree
        effects:
          - setFlag: test_accepted
      - id: refuse
        text: dlg_test.intro.choice.refuse
        to: farewell

  - id: agree
    line:
      text: dlg_test.agree.text
    autoAdvance: { ms: 1000 }
    choices:
      - id: continue
        text: dlg_common.continue
        to: $END

  - id: farewell
    line:
      text: dlg_test.farewell.text
    choices:
      - id: close
        text: dlg_common.close
        to: $END
)";

    try {
        std::istringstream goethe_stream(goethe_yaml);
        goethe::Dialogue goethe_dialogue = goethe::read_dialogue(goethe_stream);
        
        std::cout << "  ✓ Loaded GOETHE dialogue: " << goethe_dialogue.id << std::endl;
        std::cout << "  ✓ Start node: " << (goethe_dialogue.startNode ? *goethe_dialogue.startNode : "first node") << std::endl;
        std::cout << "  ✓ Nodes: " << goethe_dialogue.nodes.size() << std::endl;
        
        for (const auto& node : goethe_dialogue.nodes) {
            std::cout << "    Node: " << node.id;
            if (node.speaker) {
                std::cout << " (Speaker: " << *node.speaker << ")";
            }
            std::cout << std::endl;
            
            if (node.line) {
                std::cout << "      Line: " << node.line->text << std::endl;
                if (node.line->voice) {
                    std::cout << "      Voice: " << node.line->voice->clipId << std::endl;
                }
                if (node.line->portrait) {
                    std::cout << "      Portrait: " << node.line->portrait->id << " (" << node.line->portrait->mood << ")" << std::endl;
                }
            }
            
            if (!node.choices.empty()) {
                std::cout << "      Choices: " << node.choices.size() << std::endl;
                for (const auto& choice : node.choices) {
                    std::cout << "        - " << choice.id << ": " << choice.text << " -> " << choice.to << std::endl;
                    if (!choice.effects.empty()) {
                        std::cout << "          Effects: " << choice.effects.size() << std::endl;
                    }
                }
            }
            
            if (node.autoAdvanceMs) {
                std::cout << "      Auto-advance: " << *node.autoAdvanceMs << "ms" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "  ✗ Error loading GOETHE format: " << e.what() << std::endl;
        return 1;
    }

    // Test 3: Write and read back
    std::cout << "\n3. Testing Write/Read Cycle:" << std::endl;
    try {
        // Create a simple GOETHE dialogue
        goethe::Dialogue test_dialogue;
        test_dialogue.id = "write_test";
        test_dialogue.startNode = "start";
        
        goethe::Node start_node;
        start_node.id = "start";
        start_node.speaker = "test_speaker";
        
        goethe::Line line;
        line.text = "test.line.text";
        line.weight = 1.0f;
        start_node.line = line;
        
        goethe::Choice choice;
        choice.id = "test_choice";
        choice.text = "test.choice.text";
        choice.to = "$END";
        start_node.choices.push_back(choice);
        
        test_dialogue.nodes.push_back(start_node);
        
        // Write to string
        std::ostringstream output;
        goethe::write_dialogue(output, test_dialogue);
        std::string written_yaml = output.str();
        
        std::cout << "  ✓ Wrote dialogue to YAML" << std::endl;
        
        // Read back
        std::istringstream input(written_yaml);
        goethe::Dialogue read_back = goethe::read_dialogue(input);
        
        std::cout << "  ✓ Read back dialogue: " << read_back.id << std::endl;
        std::cout << "  ✓ Nodes: " << read_back.nodes.size() << std::endl;
        
        if (read_back.nodes.size() > 0) {
            const auto& node = read_back.nodes[0];
            std::cout << "  ✓ First node: " << node.id << std::endl;
            if (node.line) {
                std::cout << "  ✓ Line text: " << node.line->text << std::endl;
            }
            std::cout << "  ✓ Choices: " << node.choices.size() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "  ✗ Error in write/read cycle: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n✓ All tests passed successfully!" << std::endl;
    return 0;
}
