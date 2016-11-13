{
  'variables': {
    'asan%': 0,
    'werror': '',                     # Turn off -Werror in V8 build.
    'visibility%': 'hidden',          # V8's visibility setting
    'target_arch%': 'ia32',           # set v8's target architecture
    'host_arch%': 'ia32',             # set v8's host architecture
    'want_separate_host_toolset%': 0, # V8 should not build target and host
    'library%': 'static_library',     # allow override to 'shared_library' for DLL/.so builds
    'component%': 'shared_library',   # NB. these names match with what V8 expects
    'msvs_multi_core_compile': '1',   # we do enable multicore compiles, but not using the V8 way
    'python%': 'python',

    'node_tag%': '',
    'uv_library%': 'static_library',

    # Default to -O0 for debug builds.
    'v8_optimized_debug%': 0,

    # Enable disassembler for `--print-code` v8 options
    'v8_enable_disassembler': 1,

    # Don't bake anything extra into the snapshot.
    'v8_use_external_startup_data%': 0,

    # Disable V8's post-mortem debugging; frequently broken and hardly used.
    'v8_postmortem_support%': 'false',

    'conditions': [
      ['OS == "win"', {
        'os_posix': 0,
      }, {
        'os_posix': 1,
      }],
      ['OS=="mac"', {
        'clang%': 1,
      }, {
        'clang%': 0,
      }],
    ],
  },
}
