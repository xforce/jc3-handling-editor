{
	'targets': [
	{
		'target_name': 'jc3_handling_editor',
		'type': 'shared_library',
		'dependencies': [
			'hooking',
			'imgui.gyp:imgui',
		],
		'sources': [
			'src/jc3_handling_editor.cpp',
			'src/singleton.h',
		],
	},
	{
		'target_name': 'hooking',
		'type' : 'static_library',
		'sources': [
			'<!@pymod_do_main(glob-files "src/hooking/**.h")',
			'src/hooking/hooking.cpp',
		],
	},
	{
		'target_name': 'libudis86',
		'type': 'static_library',
		'sources': [
            '<!@pymod_do_main(glob-files "src/hooking/libudis86/*.h")',
            '<!@pymod_do_main(glob-files "src/hooking/libudis86/*.c")',
		],
	},
	]
}