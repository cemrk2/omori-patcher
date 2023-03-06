console.log('stdlib: initializing');

try {
	function rpc(funcId, data) {
		const obj = { 'func': funcId, 'data': data };
		const jsonTxt = JSON.stringify(obj);
		print('<omori-patcher>: ' + jsonTxt);
	}
	
	function mp_pre(name, cbName) {
		const data = {
			'name': name,
			'callback': cbName
		};
		rpc(1, data);
	}
	
	function mp_replace(name, cbName) {
		const data = {
			'name': name,
			'callback': cbName
		};
		rpc(2, data);
	}
	
	function mp_post(name, cbName) {
		const data = {
			'name': name,
			'callback': cbName
		};
		rpc(3, data);
	}
	
	function mp_commit(name) {
		const data = {
			'name': name
		};
		rpc(4, data);
	}
	
	function writeFileEx(filename, filedata, replaceExisting = true) {
		const data = {
			'filename': filename,
			'data': filedata,
			'replace': replaceExisting
		};
		rpc(5, data);
	}
	
	console.log('stdlib: initialized');
} catch (ex) {
	console.error(ex);
}