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
#define DEFAULT_TARGET "unsupported"
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

	// Checking target
	int target = -1; // TODO: enum + separate func
	if (strcmp(*f_target, "linux") == 0)   target = 0;
	if (strcmp(*f_target, "windows") == 0) target = 1;
	if (target == -1) {
		nob_log(ERROR, "No such target.");
		exit(1);
	}

	// Building

	Cmd cmd = {0};

	if (!nob_mkdir_if_not_exists("./build")) return 1;

	if (nob_needs_rebuild1(nob_temp_sprintf("build/mongoose_%s.o", *f_target), MONGOOSE"/mongoose.c")) {
		nob_cmd_append(&cmd, *f_cc, MONGOOSE"/mongoose.c", "-c", "-o");
		nob_cmd_append(&cmd, nob_temp_sprintf("build/mongoose_%s.o", *f_target));
		if (!cmd_run(&cmd)) return 1;
	}
	nob_temp_reset();

	switch (target) {
		case 0:
			nob_cmd_append(&cmd, *f_cc, "main.c");
			nob_cmd_append(&cmd, "build/mongoose_linux.o");
			nob_cmd_append(&cmd, "-I"MONGOOSE);
			//nob_cmd_append(&cmd, LINUX_RAYLIB"/lib/libraylib.a");
			//nob_cmd_append(&cmd, "-I"LINUX_RAYLIB"/include");
			nob_cmd_append(&cmd, "-o", LINUX_OUTPUT);
			nob_cmd_append(&cmd, "-lm");
			if (!cmd_run(&cmd)) return 1;
			if (*f_run) {
				nob_cmd_append(&cmd, LINUX_OUTPUT);
				for (int i = 0; i < rargc; i++) { nob_cmd_append(&cmd, rargv[i]); }
				if (!cmd_run(&cmd)) return 1;
			}
			break;
		case 1:
			nob_cmd_append(&cmd, *f_cc, "main.c");
			nob_cmd_append(&cmd, "build/mongoose_windows.o");
			nob_cmd_append(&cmd, "-I"MONGOOSE);
			//nob_cmd_append(&cmd, WINDOWS_RAYLIB"/lib/libraylib.a");
			//nob_cmd_append(&cmd, "-I"WINDOWS_RAYLIB"/include");
			nob_cmd_append(&cmd, "-o", WINDOWS_OUTPUT);
			nob_cmd_append(&cmd, "-mwindows");
			nob_cmd_append(&cmd, "-lws2_32");
			if (!cmd_run(&cmd)) return 1;
			if (*f_run) {
				nob_cmd_append(&cmd, WINDOWS_OUTPUT);
				for (int i = 0; i < rargc; i++) { nob_cmd_append(&cmd, rargv[i]); }
				if (!cmd_run(&cmd)) return 1;
			}
			break;
	}

	return 0;
}
