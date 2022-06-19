{
    "targets": [
        {
            "target_name": "before_build",
            "type": "none",
            "copies": [{
                "files": [
                    "lib/libcblite-3.0.1/lib/libcblite.3.dylib",
                ],
                "destination": "<(PRODUCT_DIR)"
            }]
        },
        {
            "target_name": "couchbaselite",
            "include_dirs": [
                "lib/libcblite-3.0.1/include",
            ],
            "sources": [
                "src/c/NapiConvert.c",
                "src/c/Listener.c",
                "src/c/util.c",
                "src/c/main.c",
            ],
            "libraries": [
                "-Wl,-rpath,@loader_path/.",
                "libcblite.3.dylib",
            ],
        }
    ]
}
