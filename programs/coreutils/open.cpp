#include <libduck/Args.h>
#include <libapp/App.h>

std::string file;
bool foreground = false;

using namespace Duck;

int main(int argc, char** argv) {
	Duck::Args args;
	args.add_positional(file, true, "APP", "The file to open.");
	args.add_flag(foreground, "f", "foreground", "Doesn't fork before executing the app, i.e. runs the app in the foreground.");
	args.parse(argc, argv);

	auto result = App::open(file, !foreground);
	if(result.is_error())
		printerrln("Couldn't open {}: {}", file, result.message());

	return result.code();
}