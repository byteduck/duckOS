extern "C" {
	int main(int argc, char** argv, char** env);

	int _start(int argc, char** argv, char** env) {
		int result = main(argc, argv, env);
		exit(result);
		return 1;
	}
}
