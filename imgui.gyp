{
	'targets': [
	{
		'target_name': 'imgui',
		'type': 'static_library',
        'include_dirs': ['deps/imgui', 'deps/imgui/examples/directx11_example'],
		'sources': [
            'deps/imgui/imconfig.h',
            'deps/imgui/imgui.h',
			'deps/imgui/imgui.cpp',
            'deps/imgui/imgui_draw.cpp',
            'deps/imgui/imgui_internal.h',
            'deps/imgui/stb_rect_pack.h',
            'deps/imgui/stb_textedit.h',
            'deps/imgui/stb_truetype.h',
            'deps/imgui/examples/directx11_example/imgui_impl_dx11.cpp',
            'deps/imgui/examples/directx11_example/imgui_impl_dx11.h',
		],
        'direct_dependent_settings': {
            'include_dirs': ['deps/imgui', 'deps/imgui/examples/directx11_example'],
         },
	}
	]
}