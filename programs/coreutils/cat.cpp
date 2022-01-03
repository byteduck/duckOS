/*
	This file is part of duckOS.

	duckOS is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	duckOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

//A program that writes the contents of a file to stdout.

#include <libduck/File.h>
#include <libduck/Args.h>
#include <libduck/Stream.h>

using Duck::File, Duck::Args, Duck::Stream, Duck::ResultRet;

std::string filename;

int main(int argc, char** argv) {
	Args args;
	args.add_positional(filename, false, "FILE", "The file to read.");
	args.parse(argc, argv);

	File file = File::std_in;

	if(!filename.empty()) {
		auto res = File::open(filename, "r");
		if(res.is_error()) {
			Stream::std_err << "cat: Couldn't open '" << filename << "': " << res.strerror() << "\n";
			return res.code();
		}
		file = res.value();
	}

	char buf[512];
	bool eof = false;
	while(true) {
		auto read_res = read(file.fd(), buf, 512);
		if(read_res < 0) {
			Stream::std_err << "cat: Couldn't read: " << strerror(errno) << "\n";
			return errno;
		}

		if(read_res == 0)
			break;

		auto write_res = write(File::std_out.fd(), buf, read_res);
		if(write_res < 0) {
			Stream::std_err << "cat: Couldn't write: " << strerror(errno) << "\n";
			return errno;
		}
	}

	return 0;
}
