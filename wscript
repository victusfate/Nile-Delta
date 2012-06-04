import Options
import glob
from os import unlink, symlink, popen
from os.path import exists, abspath

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.add_option('--profile', action='store_true', help='Enable profiling', dest='profile', default=False)

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  flags = ['-O3', '-Wall', '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE', '-msse2']

  conf.env.append_value('CCFLAGS', flags)
  conf.env.append_value('CXXFLAGS', flags)

  print flags

# def build(bld):
#   obj = bld.new_task_gen("cxx", "shlib", "node_addon")
#   obj.target = "logStream"
#   obj.source =  bld.glob('src/*.cc')
#   obj.cxxflags = ["-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE"]
#   obj.uselib = []


def shutdown():
  if Options.commands['clean']:
    if exists('logStream.node'): unlink('logStream.node')
  else:
    if exists('build/default/logStream.node') and not exists('logStream.node'):
      symlink('build/default/logStream.node', 'logStream.node')

