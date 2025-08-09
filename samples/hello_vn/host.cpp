#include "goethe.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct Choice { std::string text; std::string target; };

static void run_scene(const std::string& path);

int main() {
  const char* mounts = "{\"mounts\":[{\"path\":\"assets\",\"type\":\"dir\"}]}";
  GoetheConfig cfg = { "Hello VN", 1280, 720, 60, 0, mounts };
  GoetheEngine* eng = goethe_create(&cfg);
  if (!eng) { std::fprintf(stderr, "Failed to create engine\n"); return 1; }

  goethe_load_project(eng, "assets/project.goethe.json");

  // Minimal console runner that interprets the simple .gsc format we placed in assets
  run_scene("assets/scenes/intro.gsc");

  goethe_destroy(eng);
  return 0;
}

static void print_dialog(const std::string& who, const std::string& line) {
  std::printf("%s: %s\n", who.c_str(), line.c_str());
}

static void run_scene(const std::string& path) {
  std::ifstream f(path);
  if (!f.good()) { std::fprintf(stderr, "Missing scene: %s\n", path.c_str()); return; }
  std::string line;
  std::vector<Choice> pendingChoices;
  std::string gotoLabel;
  while (std::getline(f, line)) {
    if (line.rfind("say", 0) == 0) {
      auto first = line.find('"');
      auto second = line.find('"', first+1);
      auto third = line.find('"', second+1);
      auto fourth = line.find('"', third+1);
      if (first!=std::string::npos && second!=std::string::npos && third!=std::string::npos && fourth!=std::string::npos) {
        std::string who = line.substr(first+1, second-first-1);
        std::string text = line.substr(third+1, fourth-third-1);
        print_dialog(who, text);
      }
    } else if (line.rfind("choice", 0) == 0) {
      pendingChoices.clear();
      // read subsequent indented options until blank or EOF
      std::streampos pos;
      while (true) {
        pos = f.tellg();
        std::string opt;
        if (!std::getline(f, opt)) break;
        if (opt.empty() || opt[0] != ' ') { f.seekg(pos); break; }
        auto q1 = opt.find('"');
        auto q2 = opt.find('"', q1+1);
        auto gt = opt.find("goto", q2);
        if (q1!=std::string::npos && q2!=std::string::npos && gt!=std::string::npos) {
          std::string text = opt.substr(q1+1, q2-q1-1);
          std::string target = opt.substr(gt+5);
          pendingChoices.push_back({text, target});
        }
      }
      if (!pendingChoices.empty()) {
        std::printf("\nChoices:\n");
        for (size_t i=0;i<pendingChoices.size();++i) {
          std::printf("  %zu) %s\n", i+1, pendingChoices[i].text.c_str());
        }
        std::printf("> ");
        int pick = 1;
        std::scanf("%d", &pick);
        if (pick < 1 || (size_t)pick > pendingChoices.size()) pick = 1;
        gotoLabel = pendingChoices[(size_t)(pick-1)].target;
      }
    } else if (line.rfind("label ", 0) == 0) {
      // labels are for in-file gotos; we don’t implement jumping within file in this tiny sample
      continue;
    } else if (line.rfind("goto ", 0) == 0) {
      gotoLabel = line.substr(5);
    }
  }
  if (!gotoLabel.empty()) {
    if (gotoLabel == "rooftop") {
      run_scene("assets/scenes/rooftop.gsc");
    } else if (gotoLabel == "home") {
      run_scene("assets/scenes/home.gsc");
    }
  }
}


