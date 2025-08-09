#include <cstdio>
#include <cstring>

int story_main(int argc, char** argv);

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "Usage: goethec <story|help> [args...]\n");
    return 1;
  }
  if (std::strcmp(argv[1], "story") == 0) {
    return story_main(argc-1, argv+1);
  }
  if (std::strcmp(argv[1], "help") == 0) {
    std::printf("Commands:\n  story build|sign|verify\n");
    return 0;
  }
  std::fprintf(stderr, "Unknown command '%s'\n", argv[1]);
  return 2;
}


