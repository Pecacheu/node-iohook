{"targets": [{
	"target_name": "iohook",
	"sources": ["src/iohook.cc"],
	"include_dirs": ["../nan", "node_modules/nan", "libuiohook/dist/include"],
	"conditions": [
		['OS=="win"', { #Windows
			"libraries": ["../libuiohook/dist/lib/uiohook"]
		}, { #Not Windows
			"libraries": ["-luiohook", "-L../libuiohook/dist/lib", "-lxkbfile", "-lxkbcommon-x11", "-lxkbcommon", "-lX11-xcb", "-lxcb", "-lXinerama", "-lXt", "-lXtst", "-lX11"]
		}]
	]
}]}