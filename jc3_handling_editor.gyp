{
	'targets': [
	{
		'target_name': 'jc3_handling_editor',
		'type': 'shared_library',
		'dependencies': [
			'deps/jc3_api/jc3_api.gyp:jc3_api',
			'imgui.gyp:imgui',
		],
		'include_dirs': ['src', 'deps/boost', 'deps/ksignals'],
		'sources': [
			'src/jc3_handling_editor.cpp',
			'src/d3d11_hook.cpp',
			'src/d3d11_hook.h',
			'src/hide_from_debugger_patch.cpp',
			'src/singleton.h',
		],
	},
	]
}