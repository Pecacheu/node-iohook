const fs=require('fs'), {exec}=require('child_process'),
IODIR=__dirname+"/libuiohook/build", WIN=process.platform=="win32",
MAKE=WIN?"-DADVAPI32=%windir%/System32/advapi32.dll":"",
HELP=WIN?"":"\nTry running: sudo apt install git cmake pkg-config libx11-dev libxtst-dev libxt-dev libxinerama-dev libx11-xcb-dev libxkbcommon-dev libxkbcommon-x11-dev libxkbfile-dev";

cmd("git clone https://github.com/kwhat/libuiohook -b 1.2", __dirname, o => {
	console.log(o); fs.mkdirSync(IODIR, {recursive:true});
	cmd("cmake -S .. -DCMAKE_INSTALL_PREFIX=../dist "+MAKE, IODIR, o => {
		console.log(o); cmd("cmake --build . -j 4 -t install", IODIR, o => {
			console.log(o); console.log("DONE!");
		});
	});
});

function cmd(c,d,cb) {
	let cp=exec(c,{cwd:d},(e,so,se) => {
		if(se && se.indexOf("already exists")!=-1) e=cp.signal=0;
		if(e||cp.signal) console.log(e||se,HELP),process.exit(1); cb(so);
	});
}