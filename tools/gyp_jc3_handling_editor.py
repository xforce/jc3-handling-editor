#!/usr/bin/env python
import glob
import os
import shlex
import sys
import platform

script_dir = os.path.dirname(__file__)
jc3_handling_editor_root  = os.path.normpath(os.path.join(script_dir, os.pardir))

sys.path.insert(0, os.path.abspath(os.path.join(jc3_handling_editor_root, 'tools')))
sys.path.insert(0, os.path.join(jc3_handling_editor_root, 'tools', 'gyp', 'pylib'))
import gyp

def run_gyp(args):
  rc = gyp.main(args)
  if rc != 0:
    print 'Error running GYP'
    sys.exit(rc)

if __name__ == '__main__':
  args = sys.argv[1:]

  # GYP bug.
  if sys.platform == 'win32':
    args.append(os.path.join(jc3_handling_editor_root, 'jc3_handling_editor.gyp'))
    standalone_fn  = os.path.join(jc3_handling_editor_root, 'standalone.gypi')
    toolchain_fn  = os.path.join(jc3_handling_editor_root, 'toolchain.gypi')
    common_fn  = os.path.join(jc3_handling_editor_root, 'common.gypi')
    options_fn = os.path.join(jc3_handling_editor_root, 'config.gypi')
  else:
    args.append(os.path.join(os.path.abspath(jc3_handling_editor_root), 'jc3_handling_editor.gyp'))
    standalone_fn  = os.path.join(os.path.abspath(jc3_handling_editor_root), 'standalone.gypi')
    toolchain_fn  = os.path.join(os.path.abspath(jc3_handling_editor_root), 'toolchain.gypi')
    common_fn  = os.path.join(os.path.abspath(jc3_handling_editor_root), 'common.gypi')
    options_fn = os.path.join(os.path.abspath(jc3_handling_editor_root), 'config.gypi')

  if os.path.exists(standalone_fn):
    args.extend(['-I', standalone_fn])

  if os.path.exists(toolchain_fn):
    args.extend(['-I', toolchain_fn])

  if os.path.exists(common_fn):
    args.extend(['-I', common_fn])

  if os.path.exists(options_fn):
    args.extend(['-I', options_fn])

  args.append('--depth=' + jc3_handling_editor_root)

  #args.append('-Dcomponent=shared_library')
  #args.append('-Dlibrary=shared_library')
  gyp_args = list(args)
  
  print os.environ.get('GYP_GENERATORS')
  
  gyp_generators = os.environ.get('GYP_GENERATORS')
  #if platform.system() == 'Linux' and gyp_generators != 'ninja':
  # --generator-output defines where the Makefile goes.
  gyp_args.append('--generator-output=out')
  # -Goutput_dir defines where the build output goes, relative to the
  # Makefile. Set it to . so that the build output doesn't end up in out/out.
  gyp_args.append('-Goutput_dir=.')
  
  run_gyp(gyp_args)
