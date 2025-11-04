#include <stdbool.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"


// --- Target ---

enum Target {
	T_UNDEFINED,
	T_LINUX,
	T_WINDOWS
};

int target = T_UNDEFINED;

// --- Definitions (compiletime) ---

#ifdef __linux__
#define DEFAULT_TARGET "linux"
#elif __MINGW32__
#define DEFAULT_TARGET "windows"
#else
#define DEFAULT_TARGET "undefined"
#error "Unsupported platform."
#endif

#define MONGOOSE "./resources/mongoose-7.19"
#define QRCODEGEN "./resources/qrcodegen"

//#define LINUX_RAYLIB "./resources/raylib-5.5/linux_amd64"
#define LINUX_OUTPUT "./build/lanfia"

//#define WINDOWS_RAYLIB "./resources/raylib-5.5/win64_mingw-w64"
#define WINDOWS_OUTPUT "./build/lanfia.exe"

// --- Flags ---

typedef struct Flags {
	bool* help;
	char** cc;
	char** target;
	bool* run;
	bool* tests;
	int rargc;
	char** rargv;
} Flags;

Flags flags;

void PrintHelp(int argc, char** argv) {
	flag_print_options(stdout);
	printf("Examples:\n");
	printf("%s\n", argv[0]);
	printf("%s -target linux\n", argv[0]);
	printf("%s -cc x86_64-w64-mingw32-gcc -target windows\n", argv[0]);
	printf("%s -run -- -log-level 2\n", argv[0]);
}

void FlagsParse(int argc, char** argv) {
	flags.help = flag_bool("help", 0, "show help");
	flags.cc = flag_str("cc", "cc", "provided compiler");
	flags.target = flag_str("target", DEFAULT_TARGET, "target platform");
	flags.run = flag_bool("run", false, "run program");
	flags.tests = flag_bool("tests", false, "JS tests page");

	if (!flag_parse(argc, argv)) {
		PrintHelp(argc, argv);
		flag_print_error(stderr);
		exit(1);
	}

	if (*flags.help) {
		PrintHelp(argc, argv);
		exit(0);
	}

	flags.rargc = flag_rest_argc();
	flags.rargv = flag_rest_argv();

	// Checking target
	if (strcmp(*flags.target, "linux") == 0)   target = T_LINUX;
	if (strcmp(*flags.target, "windows") == 0) target = T_WINDOWS;
	if (target == T_UNDEFINED) {
		nob_log(ERROR, "No such target.");
		exit(1);
	}
}

// --- Build ---

Cmd cmd = {0};

void PreBuild() {
	if (!nob_mkdir_if_not_exists("./build")) exit(1);

	if (nob_needs_rebuild1(nob_temp_sprintf("build/mongoose_%s.o", *flags.target), MONGOOSE"/mongoose.c")) {
		nob_cmd_append(&cmd, *flags.cc, MONGOOSE"/mongoose.c", "-c", "-o");
		nob_cmd_append(&cmd, nob_temp_sprintf("build/mongoose_%s.o", *flags.target));
		if (!cmd_run(&cmd)) exit(1);
	}
	nob_temp_reset();

	if (nob_needs_rebuild1(nob_temp_sprintf("build/qrcodegen_%s.o", *flags.target), QRCODEGEN"/qrcodegen.c")) {
		nob_cmd_append(&cmd, *flags.cc, QRCODEGEN"/qrcodegen.c", "-c", "-o");
		nob_cmd_append(&cmd, nob_temp_sprintf("build/qrcodegen_%s.o", *flags.target));
		if (!cmd_run(&cmd)) exit(1);
	}
	nob_temp_reset();
}

void Build() {
	switch (target) {
		case T_LINUX:
			nob_cmd_append(&cmd, *flags.cc, "main.c");
			nob_cmd_append(&cmd, "build/mongoose_linux.o");
			nob_cmd_append(&cmd, "build/qrcodegen_linux.o");
			nob_cmd_append(&cmd, "-I"MONGOOSE);
			nob_cmd_append(&cmd, "-I"QRCODEGEN);
			//nob_cmd_append(&cmd, LINUX_RAYLIB"/lib/libraylib.a");
			//nob_cmd_append(&cmd, "-I"LINUX_RAYLIB"/include");
			nob_cmd_append(&cmd, "-o", LINUX_OUTPUT);
			nob_cmd_append(&cmd, "-lm");
			if (!cmd_run(&cmd)) exit(1);
			if (*flags.run || *flags.tests) {
				nob_cmd_append(&cmd, LINUX_OUTPUT);
				if (*flags.tests) nob_cmd_append(&cmd, "-tests");
				for (int i = 0; i < flags.rargc; i++) { nob_cmd_append(&cmd, flags.rargv[i]); }
				if (!cmd_run(&cmd)) exit(1);
			}
			break;
		case T_WINDOWS:
			nob_cmd_append(&cmd, *flags.cc, "main.c");
			nob_cmd_append(&cmd, "build/mongoose_windows.o");
			nob_cmd_append(&cmd, "build/qrcodegen_windows.o");
			nob_cmd_append(&cmd, "-I"MONGOOSE);
			nob_cmd_append(&cmd, "-I"QRCODEGEN);
			//nob_cmd_append(&cmd, WINDOWS_RAYLIB"/lib/libraylib.a");
			//nob_cmd_append(&cmd, "-I"WINDOWS_RAYLIB"/include");
			nob_cmd_append(&cmd, "-o", WINDOWS_OUTPUT);
			nob_cmd_append(&cmd, "-mwindows");
			nob_cmd_append(&cmd, "-lws2_32");
			if (!cmd_run(&cmd)) exit(1);
			if (*flags.run || *flags.tests) {
				nob_cmd_append(&cmd, WINDOWS_OUTPUT);
				if (*flags.tests) nob_cmd_append(&cmd, "-tests");
				for (int i = 0; i < flags.rargc; i++) { nob_cmd_append(&cmd, flags.rargv[i]); }
				if (!cmd_run(&cmd)) exit(1);
			}
			break;
	}
}

int main(int argc, char** argv) {

	NOB_GO_REBUILD_URSELF(argc, argv);

	FlagsParse(argc, argv);

	Build();

	return 0;
}
