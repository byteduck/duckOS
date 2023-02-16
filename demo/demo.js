/// <reference path="libv86.js" />

window.onload = function() {
	var screen = document.getElementById("screen_container");
	var serial = document.getElementById("serial");
	
	var options = {
		memory_size: 128 * 1024 * 1024,
		vga_memory_size: 8 * 1024 * 1024,
		autostart: true,
		bios: {
			url: 'bios/seabios.bin'
		},
		vga_bios: {
			url: 'bios/vgabios.bin'
		},
		hda: {
			url: 'duckOS-v5.img',
			async: true
		},
		initial_state: {
			url: "state-v5.bin.zst"
		},
		serial_container_xtermjs: serial,
		screen_container: screen
	};

	window.emulator = new V86Starter(options);

	var progress = document.getElementById("progress");
	var progressLabel = document.getElementById("progress_label");
	window.emulator.add_listener("download-progress", function(e) {
		if(e.lengthComputable) {
			progress.max = e.total;
			progress.value = e.loaded;
			progressLabel.innerHTML = "Downloading " + e.file_name;
		} else {
			progress.max = undefined;
		}
	});
	window.emulator.add_listener("download-error", function(e) {
		console.log(e);
		progressLabel.innerHTML = "Error downloading " + e.file_name;
	});
	window.emulator.add_listener("emulator-ready", function(e) {
		document.getElementById("progress_container").style.display = "none";
		document.getElementById("emulator").style.visibility = "visible";
	});
	
	var loadIndicator = document.querySelector("#load-indicator");
	window.emulator.add_listener("ide-read-start", function(e) {
		loadIndicator.classList.add("on");
	});
	window.emulator.add_listener("ide-read-end", function(e) {
		loadIndicator.classList.remove("on");
	});

	screen.onclick = function() {
		window.emulator.lock_mouse();
	}

}

window.downloadState = async function() {
	let state = await emulator.save_state()
	var a = document.createElement("a");
	a.download = "duckOS-state.bin";
	a.href = window.URL.createObjectURL(new Blob([state]));
	a.dataset.downloadurl = "application/octet-stream:" + a.download + ":" + a.href;
	a.click();
}

window.fullscreen = function() {
	document.getElementById("screen_container").requestFullscreen();
}
