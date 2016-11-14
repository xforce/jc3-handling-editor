{
	'targets': [
	{
		'target_name': 'jc3_handling_editor',
		'type': 'shared_library',
		'dependencies': [
			'deps/jc3_api/jc3_api.gyp:jc3_api',
			'imgui.gyp:imgui',
		],
		'include_dirs': ['src', 'deps/boost'],
		'sources': [
			'src/jc3_handling_editor.cpp',
			'src/singleton.h',
		],
	},
	]
}