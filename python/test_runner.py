import xml.etree.ElementTree as ET
import sys,argparse,os,subprocess
from pathlib import Path
from re import compile
from console_progressbar import ProgressBar
from shutil import copyfile
from datetime import datetime
import matplotlib.pyplot as plt

def deleteFile(fname):
	try:
		os.remove(fname)
	except FileNotFoundError:
		pass

class XMLFile:
	def __init__(self, filename=None, root=None):
		if filename is not None:
			self.et = ET.parse(str(filename))
			self.root = self.et.getroot()
		else:
			self.et = None
			self.root = root

	def get(self, path, attrib=None):
		xe = self.root.find(path)
		if xe is None : return None
		return xe.text if attrib is None else xe.attrib[attrib]

	def set(self, path, **kwargs):
		xe = self.root.find(path)
		if xe is None:
			xe = self.root
			for xn in path.split('/'):
				sn = xe.find(xn)
				xe = ET.SubElement(xe,xn) if sn is None else sn
		if len(kwargs)==1 and 'value' in kwargs:
			xe.text = str(kwargs['value'])
		else:
			for k,n in kwargs.items():
				xe.attrib[k] = str(n)

	def remove(self, path, *args):
		xe = self.root.find(path)
		if xe is None: return
		for name in args:
			if name in xe.attrib:
				del xe.attrib[name]
	
	def find(self, path):
		return self.root.find(path)
	
	def findall(self, path):
		return self.root.findall(path)
	
	def save(self,filename):
		self.et.write(filename)
	
def main(Args):
	pass

def test():
	pass

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('-test', 	action='store_true')
	parser.add_argument('config',  	type=Path, help="xml (CalcIVD) design files to run")
	parser.add_argument('-results',type=Path, help="result folder")
	parser.add_argument('-bin',	type=Path, default="c:\\MyData\\Projects\\gra_w_pana\\c++\\x64\\Release", help='bin directory')
	Args = parser.parse_args(sys.argv[1:])

	if Args.test:
		test()
	else:
		main(Args)
