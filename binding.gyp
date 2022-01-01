{
    "targets": [
        {
            "target_name": "before_build",
            "type": "none",
            "copies": [{
                "files": [
                    "lib/libcblite-3.0.0/lib/libcblite.3.dylib",
                ],
                "destination": "<(PRODUCT_DIR)"
            }]
        },
        {
            "target_name": "couchbaselite",
            "include_dirs": [
                "lib/libcblite-3.0.0/include",
            ],
            "sources": [
                "src/main.c",
            ],

            "libraries": [
                "-Wl,-rpath,@loader_path/.",
                "libcblite.3.dylib",
            ],
        }
    ]
}
