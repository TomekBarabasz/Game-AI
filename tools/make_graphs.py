from pathlib import Path
from subprocess import run
import sys, argparse
from os import getcwd,remove

parser = argparse.ArgumentParser()
parser.add_argument('-file',		type=Path, nargs='+', help="file to convert")
parser.add_argument('-folder',		type=Path, default='.',help="working folder")
parser.add_argument('-exe','-e',	type=str, default='dot.exe', help="graph creator executable like dot.exe or twopi.exe")
parser.add_argument('-force','-f',	action="store_true", help="use to overwrite existing svg files")
parser.add_argument('-rem','-r',	action="store_true", help="use to remove gv files after converting")
Args = parser.parse_args(sys.argv[1:])

def convertFile(path, exe, force):
	if path.suffix == '.gv':
		out = path.parent / (path.stem + '.svg')
		if force or not out.exists():
			run( [exe, '-Tsvg', str(path), '-o', str(out), '-Goverlap=prism'])
			print( exe, '-Tsvg', str(path), '-o', str(out), '-Goverlap=prism')
			if Args.rem:
				remove(path)

if Args.file is not None:
	for file in Args.file:
		convertFile(file, Args.exe, True)
else:
	for e in Args.folder.iterdir():
		if e.is_file():
			convertFile(e, Args.exe, Args.force)
