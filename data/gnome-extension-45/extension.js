import * as Main from 'resource:///org/gnome/shell/ui/main.js';
import GLib from 'gi://GLib';
import Shell from 'gi://Shell';
import Gio from 'gi://Gio';
import { Extension } from 'resource:///org/gnome/shell/extensions/extension.js';

let file = Gio.File.new_for_path(makePipe());
let pipe = file.append_to_async(0, 0, null, on_pipe_open);

function makePipe() {
	let runtime_dir = GLib.getenv('XDG_RUNTIME_DIR');
	if (!runtime_dir)
		runtime_dir = '/run/user/'+new TextDecoder().decode(
			GLib.spawn_command_line_sync('id -u')[1]
		).trim();

	let path = runtime_dir + '/keyd.fifo';
	GLib.spawn_command_line_sync('mkfifo ' + path);

	return path;
}

function send(msg) {
	if (!pipe)
		return;

	try {
		pipe.write(msg, null);
	} catch {
		log('pipe closed, reopening...');
		pipe = null;
		file.append_to_async(0, 0, null, on_pipe_open);
	}
}

function on_pipe_open(file, res) {
	log('pipe opened');
	pipe = file.append_to_finish(res);
}

export default class KeydExtension extends Extension {
	enable() {
		Shell.WindowTracker.get_default().connect('notify::focus-app', () => {
			const win = global.display.focus_window;
			const cls = win ? win.get_wm_class() : 'root';
			const title = win ? win.get_title() : '';

			send(`${cls}	${title}\n`);
		});

		Main.layoutManager.connectObject(
			'system-modal-opened', () => {
				send(`system-modal	${global.stage.get_title()}\n`);
			},
			this
		);

		 GLib.spawn_command_line_async('keyd-application-mapper -d'); 
	}

	disable() {
		GLib.spawn_command_line_async('pkill -f keyd-application-mapper');
	}
}

