import optparse
import os
import subprocess
import sys
import multiprocessing
import json
from pprint import pprint

from sys import platform as _platform

def LoadPythonDictionary(path):
  file_string = open(path).read()
  try:
    file_data = eval(file_string, {'__builtins__': None}, None)
  except SyntaxError, e:
    e.filename = path
    raise
  except Exception, e:
    raise Exception("Unexpected error while reading %s: %s" % (path, str(e)))

  assert isinstance(file_data, dict), "%s does not eval to a dictionary" % path

  # Flatten any variables to the top level.
  if 'variables' in file_data:
    file_data.update(file_data['variables'])
    del file_data['variables']

  # Strip any conditions.
  if 'conditions' in file_data:
    del file_data['conditions']
  if 'target_conditions' in file_data:
    del file_data['target_conditions']

  # Strip targets in the toplevel, since some files define these and we can't
  # slurp them in.
  if 'targets' in file_data:
    del file_data['targets']

  return file_data


def ReplaceSubstrings(values, search_for, replace_with):
  """Recursively replaces substrings in a value.

  Replaces all substrings of the "search_for" with "repace_with" for all
  strings occurring in "values". This is done by recursively iterating into
  lists as well as the keys and values of dictionaries."""
  if isinstance(values, str):
    return values.replace(search_for, replace_with)

  if isinstance(values, list):
    return [ReplaceSubstrings(v, search_for, replace_with) for v in values]

  if isinstance(values, dict):
    # For dictionaries, do the search for both the key and values.
    result = {}
    for key, value in values.items():
      new_key = ReplaceSubstrings(key, search_for, replace_with)
      new_value = ReplaceSubstrings(value, search_for, replace_with)
      result[new_key] = new_value
    return result

  # Assume everything else is unchanged.
  return values

try:
    from _winreg import *
except ImportError:
    print "No Windows"

parser = optparse.OptionParser()
parser.add_option('--debug',
    action='store_true',
    dest='debug',
    help='build in debug [Without its building release]')
    
parser.add_option('--builddir',
    action='store',
    dest='builddir',
    help='specifies the builddir for gyp')

(options, args) = parser.parse_args()

def GetMSBuildPath():
    aReg = ConnectRegistry(None,HKEY_LOCAL_MACHINE)

    aKey = OpenKey(aReg, r"SOFTWARE\Microsoft\MSBuild\ToolsVersions\14.0")
    try:
        val=QueryValueEx(aKey, "MSBuildToolsPath")[0] + "MSBuild.exe"
        return val
    except EnvironmentError:
        print "Unable to find msbuild"
        return ""

def execute(argv, env=os.environ):
  try:
    subprocess.check_call(argv, env=env)
    return 0
  except subprocess.CalledProcessError as e:
    return e.returncode
  raise e

def execute_stdout(argv, env=os.environ):
  try:
    output = subprocess.check_output(argv, stderr=subprocess.STDOUT, env=env)
    return output
  except subprocess.CalledProcessError as e:
    print e.output
    raise e

def RunMSBuild(configuration, builddir=""):
    gypi_values = LoadPythonDictionary('config.gypi')
    
    msbuild = GetMSBuildPath()
    if msbuild == "":
        return
    platform = "Win32"
    if gypi_values['target_arch'] == "x64":
        platform = "x64"
        
    var = execute([msbuild, "out/jc3_handling_editor.sln", "/t:Build", "/p:Configuration=" + configuration,  "/p:Platform=" + platform])
    return var

def RunMake(configuration, builddir=""):
    env = os.environ.copy()
    env['BUILDTYPE'] = configuration
    
    if builddir:
      env['builddir'] = builddir
    
    var = execute(["make","-C", "out", "-j" + str(multiprocessing.cpu_count())], env)
    return var

if _platform == "win32":
    if options.debug:
        sys.exit(RunMSBuild("Debug"))
    else:
        sys.exit(RunMSBuild("Release"))

if _platform.startswith("linux") or _platform.startswith("darwin"):
    if options.debug:
        sys.exit(RunMake("Debug", options.builddir))
    else:
        sys.exit(RunMake("Release", options.builddir))