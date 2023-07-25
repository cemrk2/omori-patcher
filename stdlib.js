console.log('stdlib: initializing');

try {
	const RPC_ID = {
		'readFileEx': 0,
		'writeFileEx': 1,
		'mkdirEx': 2,
		'getAsyncKeyState': 3,
		'messageBox': 4,
		'getClipboard': 5
	};
	let interval_i = 0;
	const intervals = [];

	/**
	 * @param {number} funcId
	 * @param {any} data
	 */
	function rpc(funcId, data) {
		const obj = { 'func': funcId, 'data': data };
		const jsonTxt = JSON.stringify(obj);
		print('<omori-patcher>: ' + jsonTxt);
	}

	/**
	 * @param {string} filename
	 * @param {string} cb_name
	 */
	function readFileEx(filename, cb_name) {
		const data = {
			'filename': filename,
			'function': cb_name
		};
		rpc(RPC_ID.readFileEx, data);
	}

	/**
	 * @param {string} filename
	 * @param {any} filedata
	 * @param {boolean} replaceExisting
	 */
	function writeFileEx(filename, filedata, replaceExisting = true) {
		const data = {
			'filename': filename,
			'data': filedata,
			'replace': replaceExisting
		};
		rpc(RPC_ID.writeFileEx, data);
	}

	/**
	 * @param {string} dirname
	 */
	function mkdirEx(dirname) {
		const data = {
			'dirname': dirname
		};
		rpc(RPC_ID.mkdirEx, data);
	}

	/**
	 * @returns {number}
	 * @param {Function} cb
	 * @param {number} interval
	 */
	function setInterval(cb, interval) {
		const id = interval_i;
		interval_i++;
		intervals.push(id);
		function wrapper() {
			if (!intervals.includes(id)) return;

			cb();
			setTimeout(wrapper, interval);
		}
		setTimeout(wrapper, interval);
		return id;
	}

	/**
	 * @param {number} id
	 */
	function clearInterval(id) {
		const i = intervals.indexOf(id);
		if (i !== -1) intervals.splice(index, 1);
	}

	/**
	 * @returns {boolean[]}
	 * @param {number[]} keycodes
	 * @param {string} cb_name
	 */
	function getAsyncKeyState(keycodes, cb_name) {
		const data = {
			'keys': keycodes,
			'function': cb_name
		}
		rpc(RPC_ID.getAsyncKeyState, data);
	}

	/**
	 * @param {string} body
	 * @param {string} title
	 */
	function messageBox(body, title) {
		const data = {
			'title': title,
			'body': body
		}
		rpc(RPC_ID.messageBox, data);
	}

	/**
	 * @param {string} cb_name
	 */
	function getClipboard(cb_name) {
		const data = {
			'function': cb_name
		}
		rpc(RPC_ID.getClipboard, data);
	}
	
	console.log('stdlib: initialized');
} catch (ex) {
	console.error(ex);
}