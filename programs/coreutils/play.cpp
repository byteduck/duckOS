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
#include <libsound/Connection.h>
#include <libsound/WavReader.h>
#include <libduck/FormatStream.h>

using Duck::File, Duck::Args, Duck::Stream, Duck::ResultRet;

std::string filename;

int main(int argc, char** argv) {
	Args args;
	args.add_positional(filename, true, "FILE", "The file to play.");
	args.parse(argc, argv);

	auto wav_res = Sound::WavReader::open_wav(filename);
	if(wav_res.is_error()) {
		Duck::printerrln("Couldn't read audio file: {}", wav_res.message());
		return wav_res.code();
	}
	auto& wav = wav_res.value();

	auto conn_res = Sound::Connection::create();
	if(conn_res.is_error()) {
		Stream::std_err << "play: Couldn't create sound server connection: " << conn_res.strerror() << "\n";
		return conn_res.code();
	}
	auto conn = conn_res.value();

	auto buf = Sound::SampleBuffer::make(wav.sample_rate(), 512);
	while(true) {
		auto res = wav.read_samples(buf);
		if(res.is_error() || res.value() == 0)
			break;
		conn->queue_samples(buf);
	}

	return 0;
}
