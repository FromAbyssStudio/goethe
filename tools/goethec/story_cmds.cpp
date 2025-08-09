#include <cstdio>
#include <cstring>

static int cmd_build(int argc, char** argv) {
  (void)argc; (void)argv;
  std::puts("[goethec] story build (stub): converts YAML -> canonical JSON -> bytecode, signs");
  return 0;
}

static int cmd_sign(int argc, char** argv) {
  (void)argc; (void)argv;
  std::puts("[goethec] story sign (stub): Ed25519 over JCS bytes");
  return 0;
}

static int cmd_verify(int argc, char** argv) {
  (void)argc; (void)argv;
  std::puts("[goethec] story verify (stub): checks digest + signature");
  return 0;
}

int story_main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "Usage: goethec story <build|sign|verify> [args...]\n");
    return 1;
  }
  const char* sub = argv[1];
  if (std::strcmp(sub, "build") == 0) return cmd_build(argc-1, argv+1);
  if (std::strcmp(sub, "sign") == 0)  return cmd_sign(argc-1, argv+1);
  if (std::strcmp(sub, "verify") == 0) return cmd_verify(argc-1, argv+1);
  std::fprintf(stderr, "Unknown subcommand '%s'\n", sub);
  return 2;
}


