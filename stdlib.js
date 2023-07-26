console.log('stdlib: initializing');

try {
	globalThis.rpc_callbacks = {};
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

	function rpc_cb(cb) {
		const cb_str = cb.toString();
		let hash = 0, i, chr;
		if (cb_str.length === 0) return hash;
		for (i = 0; i < cb_str.length; i++) {
			chr = cb_str.charCodeAt(i);
			hash = ((hash << 5) - hash) + chr;
			hash |= 0;
		}
		const hash_str = hash < 0 ? `n${Math.abs(hash)}` : `p${hash}`;
		globalThis.rpc_callbacks[hash_str] = cb;

		return hash_str;
	}

	/**
	 * @param {string} filename
	 * @param {Function} cb
	 */
	function readFileEx(filename, cb) {
		const data = {
			'filename': filename,
			'function': rpc_cb(cb)
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
	 * @param {Function} cb
	 */
	function getAsyncKeyState(keycodes, cb) {
		const data = {
			'keys': keycodes,
			'function': rpc_cb(cb)
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
	 * @param {Function} cb
	 */
	function getClipboard(cb) {
		const data = {
			'function': rpc_cb(cb)
		}
		rpc(RPC_ID.getClipboard, data);
	}
	
	console.log('stdlib: initialized');
} catch (ex) {
	console.error(ex);
}