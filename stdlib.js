console.log('stdlib: initializing');

try {
	let cb_i = 0;
	function rpc(funcId, data) {
		const obj = { 'func': funcId, 'data': data };
		const jsonTxt = JSON.stringify(obj);
		print('<omori-patcher>: ' + jsonTxt);
	}

	function writeFileEx(filename, filedata, replaceExisting = true) {
		const data = {
			'filename': filename,
			'data': filedata,
			'replace': replaceExisting
		};
		rpc(1, data);
	}

	function mkdirEx(dirname) {
		const data = {
			'dirname': dirname
		};
		rpc(2, data);
	}

	function GetAsyncKeyState(keycodes, cb) {
		cb_i++;
		window[`ml_callback_${cb_i}`] = cb;
		const data = {
			'keys': keycodes,
			'function': `ml_callback_${cb_i}`
		}
		rpc(3, data);
	}
	
	console.log('stdlib: initialized');
} catch (ex) {
	console.error(ex);
}