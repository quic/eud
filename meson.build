project('eud',
        'cpp',
        version: '0.1',
        # default_options: ['warning_level=1', 'buildtype=release']
)
libeud_srcs = [
        'src/eud_error_defines.cpp',
        'src/usb.cpp',
        'src/device_manager.cpp',
        'src/general_api_processing.cpp',
        'src/rw_childprocess.cpp',
        'src/eud.cpp',
        'src/eud_api.cpp',
        'src/jtag_eud.cpp',
        'src/jtag_api.cpp',
        'src/jtag_test.cpp',
        'src/com_eud.cpp',
        'src/com_api.cpp',
        'src/ctl_eud.cpp',
        'src/ctl_api.cpp',
        'src/trc_eud.cpp',
        'src/trc_api.cpp',
        'src/swd_eud.cpp',
        'src/swd_api.cpp',
]

if build_machine.system() == 'windows'
        shared_library('eud',
                       libeud_srcs,
                       version: meson.project_version(),
                       dependencies: dependency('libusb-1.0'),
                       include_directories : include_directories('inc'),
        )
else
        static_library('eud',
                       libeud_srcs,
                       dependencies: dependency('libusb-1.0'),
                       include_directories : include_directories('inc'),
                       cpp_args: '-Wno-narrowing', # TODO: drop me
        )
endif
