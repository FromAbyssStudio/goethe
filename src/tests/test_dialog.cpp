#include "goethe/dialog.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <sstream>

class DialogTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for all tests
    }
    
    void TearDown() override {
        // Common cleanup for all tests
    }
};

// Test fixture for simple format tests
class SimpleFormatTest : public DialogTest {
protected:
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
};

// Test fixture for GOETHE format tests
class GoetheFormatTest : public DialogTest {
protected:
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
          - type: SET_FLAG
            target: test_accepted
            value: true
      - id: refuse
        text: dlg_test.intro.choice.refuse
        to: farewell

  - id: agree
    line:
      text: dlg_test.agree.text
    autoAdvanceMs: 1000
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
};

// Simple format tests
TEST_F(SimpleFormatTest, LoadSimpleDialogue) {
    std::istringstream stream(simple_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    EXPECT_EQ(dialogue.id, "test_simple");
    EXPECT_EQ(dialogue.nodes.size(), 2);
}

TEST_F(SimpleFormatTest, SimpleDialogueNodes) {
    std::istringstream stream(simple_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    ASSERT_EQ(dialogue.nodes.size(), 2);
    
    // Check first node
    const auto& greeting_node = dialogue.nodes[0];
    EXPECT_EQ(greeting_node.id, "greeting");
    EXPECT_TRUE(greeting_node.speaker.has_value());
    EXPECT_EQ(*greeting_node.speaker, "alice");
    EXPECT_TRUE(greeting_node.line.has_value());
    EXPECT_EQ(greeting_node.line->text, "Hello from simple format!");
    
    // Check second node
    const auto& response_node = dialogue.nodes[1];
    EXPECT_EQ(response_node.id, "response");
    EXPECT_TRUE(response_node.speaker.has_value());
    EXPECT_EQ(*response_node.speaker, "bob");
    EXPECT_TRUE(response_node.line.has_value());
    EXPECT_EQ(response_node.line->text, "This is a simple dialogue.");
}

TEST_F(SimpleFormatTest, SimpleDialogueNoStartNode) {
    std::istringstream stream(simple_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    // Simple format doesn't specify startNode, so it should be nullopt
    EXPECT_FALSE(dialogue.startNode.has_value());
}

// GOETHE format tests
TEST_F(GoetheFormatTest, LoadGoetheDialogue) {
    std::istringstream stream(goethe_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    EXPECT_EQ(dialogue.id, "test_goethe");
    EXPECT_EQ(dialogue.nodes.size(), 3);
    EXPECT_TRUE(dialogue.startNode.has_value());
    EXPECT_EQ(*dialogue.startNode, "intro");
}

TEST_F(GoetheFormatTest, GoetheDialogueNodes) {
    std::istringstream stream(goethe_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    ASSERT_EQ(dialogue.nodes.size(), 3);
    
    // Check intro node
    const auto& intro_node = dialogue.nodes[0];
    EXPECT_EQ(intro_node.id, "intro");
    EXPECT_TRUE(intro_node.speaker.has_value());
    EXPECT_EQ(*intro_node.speaker, "marshal");
    EXPECT_TRUE(intro_node.line.has_value());
    EXPECT_EQ(intro_node.line->text, "dlg_test.intro.text");
    
    // Check choices
    ASSERT_EQ(intro_node.choices.size(), 2);
    
    const auto& accept_choice = intro_node.choices[0];
    EXPECT_EQ(accept_choice.id, "accept");
    EXPECT_EQ(accept_choice.text, "dlg_test.intro.choice.accept");
    EXPECT_EQ(accept_choice.to, "agree");
    
    const auto& refuse_choice = intro_node.choices[1];
    EXPECT_EQ(refuse_choice.id, "refuse");
    EXPECT_EQ(refuse_choice.text, "dlg_test.intro.choice.refuse");
    EXPECT_EQ(refuse_choice.to, "farewell");
}

TEST_F(GoetheFormatTest, GoetheDialogueEffects) {
    std::istringstream stream(goethe_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    const auto& intro_node = dialogue.nodes[0];
    const auto& accept_choice = intro_node.choices[0];
    
    ASSERT_EQ(accept_choice.effects.size(), 1);
    
    const auto& effect = accept_choice.effects[0];
    EXPECT_EQ(effect.type, goethe::Effect::Type::SET_FLAG);
    EXPECT_EQ(effect.target, "test_accepted");
    EXPECT_TRUE(std::holds_alternative<bool>(effect.value));
    EXPECT_EQ(std::get<bool>(effect.value), true);
}

TEST_F(GoetheFormatTest, GoetheDialogueAutoAdvance) {
    std::istringstream stream(goethe_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    const auto& agree_node = dialogue.nodes[1];
    EXPECT_TRUE(agree_node.autoAdvanceMs.has_value());
    EXPECT_EQ(*agree_node.autoAdvanceMs, 1000);
}

TEST_F(GoetheFormatTest, GoetheDialogueEndNode) {
    std::istringstream stream(goethe_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    const auto& agree_node = dialogue.nodes[1];
    const auto& continue_choice = agree_node.choices[0];
    EXPECT_EQ(continue_choice.to, "$END");
    
    const auto& farewell_node = dialogue.nodes[2];
    const auto& close_choice = farewell_node.choices[0];
    EXPECT_EQ(close_choice.to, "$END");
}

// Error handling tests
TEST_F(DialogTest, InvalidYamlThrowsException) {
    std::string invalid_yaml = R"(
id: test_invalid
nodes:
  - id: greeting
    speaker: alice
    line:
      text: "Missing closing quote
      invalid: [unclosed: array
      malformed: {unclosed: object
      syntax: error: : : : : :
      completely: broken: yaml: structure: here
      invalid_yaml: [unclosed: array: with: colons: inside
      tab:	character: here
      invalid: [unclosed: array: with: colons: inside: and: more: colons: here
      missing: closing: quote: and: more: broken: syntax: here
      unclosed: quote: "this is not closed
)";
    
    std::istringstream stream(invalid_yaml);
    EXPECT_THROW(goethe::read_dialogue(stream), std::exception);
}

TEST_F(DialogTest, EmptyYamlThrowsException) {
    std::string empty_yaml = "";
    std::istringstream stream(empty_yaml);
    EXPECT_THROW(goethe::read_dialogue(stream), std::exception);
}

TEST_F(DialogTest, MissingIdThrowsException) {
    std::string missing_id_yaml = R"(
nodes:
  - id: greeting
    speaker: alice
    line:
      text: "No ID specified"
)";
    
    std::istringstream stream(missing_id_yaml);
    EXPECT_THROW(goethe::read_dialogue(stream), std::exception);
}

// Edge case tests
TEST_F(DialogTest, EmptyNodesList) {
    std::string empty_nodes_yaml = R"(
id: test_empty_nodes
nodes: []
)";
    
    std::istringstream stream(empty_nodes_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    EXPECT_EQ(dialogue.id, "test_empty_nodes");
    EXPECT_EQ(dialogue.nodes.size(), 0);
}

TEST_F(DialogTest, NodeWithoutSpeaker) {
    std::string no_speaker_yaml = R"(
id: test_no_speaker
nodes:
  - id: narration
    line:
      text: "This is narration without a speaker"
)";
    
    std::istringstream stream(no_speaker_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    ASSERT_EQ(dialogue.nodes.size(), 1);
    const auto& node = dialogue.nodes[0];
    EXPECT_EQ(node.id, "narration");
    EXPECT_FALSE(node.speaker.has_value());
    EXPECT_TRUE(node.line.has_value());
    EXPECT_EQ(node.line->text, "This is narration without a speaker");
}

TEST_F(DialogTest, NodeWithoutLine) {
    std::string no_line_yaml = R"(
id: test_no_line
nodes:
  - id: choice_only
    speaker: alice
    choices:
      - id: option1
        text: "Option 1"
        to: next
)";
    
    std::istringstream stream(no_line_yaml);
    goethe::Dialogue dialogue = goethe::read_dialogue(stream);
    
    ASSERT_EQ(dialogue.nodes.size(), 1);
    const auto& node = dialogue.nodes[0];
    EXPECT_EQ(node.id, "choice_only");
    EXPECT_TRUE(node.speaker.has_value());
    EXPECT_EQ(*node.speaker, "alice");
    EXPECT_FALSE(node.line.has_value());
    EXPECT_EQ(node.choices.size(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
