#include <libduck/Args.h>
#include <libapp/App.h>
#include <cstring>
#include <unistd.h>

std::string app_name;
bool background = false;

int main(int argc, char** argv) {
    Duck::Args args;
    args.add_positional(app_name, true, "APP", "The app to open. Can either be a name without the .app extension, or the path to an app.");
    args.add_flag(background, "b", "background", "Forks before executing the app, i.e. runs the app in the background.");
    args.parse(argc, argv);

    ResultRet<App::Info> res = Result::FAILURE;
    if(app_name.find('/') != std::string::npos)
        res = App::Info::from_app_directory(app_name);
    else
        res = App::Info::from_app_name(app_name);

    if(res.is_error()) {
        printf("Could not find app %s\n", app_name.c_str());
        return res.code();
    }

    auto& app = res.value();

    if(!background || !fork()) {
        char *app_argv[] = {NULL};
        char *app_envp[] = {NULL};
        execve(app.exec().c_str(), app_argv, app_envp);

        printf("Could not run app executable %s: %s\n", app.exec().c_str(), strerror(errno));
        return -1;
    }

    return 0;
}