#include <stdbool.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"

#ifdef __linux__
#define DEFAULT_TARGET "linux"
#elif __MINGW32__
#define DEFAULT_TARGET "windows"
#else
#error "Unsupported platform."
#endif

#define MONGOOSE "./resources/mongoose-7.19"

//#define LINUX_RAYLIB "./resources/raylib-5.5/linux_amd64"
#define LINUX_OUTPUT "./build/lanfia"

//#define WINDOWS_RAYLIB "./resources/raylib-5.5/win64_mingw-w64"
#define WINDOWS_OUTPUT "./build/lanfia.exe"

int main(int argc, char** argv) {

	NOB_GO_REBUILD_URSELF(argc, argv);
	
	bool* f_help = flag_bool("help", 0, "show help");
	char** f_cc = flag_str("cc", "cc", "provided compiler");
	char** f_target = flag_str("target", DEFAULT_TARGET, "target platform");
	bool* f_run = flag_bool("run", false, "run program");

	if (!flag_parse(argc, argv)) {
		flag_print_options(stdout);
		flag_print_error(stderr);
		exit(1);
	}

	if (*f_help) {
    flag_print_options(stdout);
		exit(0);
	}

	// Building

	Cmd cmd = {0};

	if (!nob_mkdir_if_not_exists("./build")) return 1;


	if (nob_needs_rebuild1("build/mongoose.o", MONGOOSE"/mongoose.c")) {
		nob_cmd_append(&cmd, *f_cc, MONGOOSE"/mongoose.c", "-c", "-o", "build/mongoose.o");
		if (!cmd_run(&cmd)) return 1;
	}

	if (strcmp(*f_target, "linux") == 0) {
		nob_cmd_append(&cmd, *f_cc, "main.c");
		nob_cmd_append(&cmd, "build/mongoose.o");
		nob_cmd_append(&cmd, "-I"MONGOOSE);
		//nob_cmd_append(&cmd, LINUX_RAYLIB"/lib/libraylib.a");
		//nob_cmd_append(&cmd, "-I"LINUX_RAYLIB"/include");
		nob_cmd_append(&cmd, "-o", LINUX_OUTPUT);
		nob_cmd_append(&cmd, "-lm");
		if (!cmd_run(&cmd)) return 1;
		if (*f_run) {
			nob_cmd_append(&cmd, LINUX_OUTPUT);
			if (!cmd_run(&cmd)) return 1;
		}
	} else if (strcmp(*f_target, "windows") == 0) {
		nob_cmd_append(&cmd, *f_cc, "main.c");
		nob_cmd_append(&cmd, "build/mongoose.o");
		nob_cmd_append(&cmd, "-I"MONGOOSE);
		//nob_cmd_append(&cmd, WINDOWS_RAYLIB"/lib/libraylib.a");
		//nob_cmd_append(&cmd, "-I"WINDOWS_RAYLIB"/include");
		nob_cmd_append(&cmd, "-o", WINDOWS_OUTPUT);
		nob_cmd_append(&cmd, "-mwindows");
		nob_cmd_append(&cmd, "-lws2_32");
		if (!cmd_run(&cmd)) return 1;
		if (*f_run) {
			nob_cmd_append(&cmd, WINDOWS_OUTPUT);
			if (!cmd_run(&cmd)) return 1;
		}
	}

	return 0;
}
