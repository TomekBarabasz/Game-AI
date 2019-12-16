import xml.etree.ElementTree as ET
import sys,argparse,os,subprocess
from pathlib import Path
from re import compile
from console_progressbar import ProgressBar
from shutil import copyfile
from datetime import datetime
import matplotlib.pyplot as plt
import operator

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
class RunConfig:
	pass
class RunResult:
	pass
class AxisDescription:
	pass

def main(Args):
	if Args.executed is None:
		resultsDir = Args.results / datetime.isoformat(datetime.now()).replace(':','_')
		resultsDir.mkdir()
	else:
		resultsDir = Args.results / Args.executed
	
	xcfg = ET.parse( Args.config )
	doRuns(xcfg, resultsDir, Args)
	show = doPlotRuns(xcfg, resultsDir, Args)
	if show:
		plt.show()

def doRuns(xcfg, resultsDir, Args):
	for xrun in xcfg.getroot().findall('run'):
		use, runConfig = prepareRun(XMLFile(root=xrun), resultsDir)
		if use:
			doRun(runConfig, bindir=Args.bin)

def doPlotRuns(xcfg, resultsDir, Args):
	show = False
	for xrun in xcfg.getroot().findall('plotrun'):
		use,runConfig = preparePlotRun(XMLFile(root=xrun), resultsDir)
		if use:
			rets = doPlotRun(runConfig, Args.bin, Args)
			plotRunResults(runConfig, rets)
			show = True
	return show

def doPlotRun(runConfig, bindir, Args):
	Results = []
	baseRunName = runConfig.run_name
	xml = XMLFile(runConfig.run_config)
	for xv in runConfig.x.values:
		path, attr = runConfig.x.var.split('#')
		xml.set(path, **{attr:xv})
		if not Args.executed:
			xml.save(runConfig.run_config)
		runConfig.run_name = baseRunName + " with {0}={1}".format(attr,xv)
		doRun(runConfig, bindir)
		orig = runConfig.results
		new = orig.parent / (orig.stem + "_{0}{1}".format(xv, orig.suffix))
		if not Args.executed:
			orig.rename(new)
		Results.append(new)
	return Results

def plotRunResults(runConfig, results):
	X = runConfig.x.values
	for rcy in runConfig.Y:
		Y = readAxisData(rcy, [(rcy, ret) for ret in results])
		fig = plt.figure()
		ax = fig.add_subplot(111)
		ax.set_xlabel(runConfig.x.label)
		ax.set_ylabel(rcy.label)
		ax.set_title(rcy.label)
		ax.plot(X,Y,"+-")
		if rcy.save is not None:
			fn = results[0].parent / (rcy.save+'.png')
			fig.savefig(str(fn))

def prepareRun(xRun, resultsFolder):
	runConfig = RunConfig()
	use = xRun.get('use')
	if use is None: use = True
	else : use = use.lower() == 'yes'
	if not use:
		return use, runConfig
	
	run_config_template = xRun.get('run_config')
	runConfig.run_config = resultsFolder / "run_config.xml"
	runConfig.results = resultsFolder / "results.xml"
	runConfig.run_name = xRun.get('name')
	runConfig.command_line = xRun.get('command_line') + ' --xml "' + str(runConfig.run_config) + '" -q' 

	#create temporary run_config
	xml = XMLFile(filename=run_config_template)
	xml.set('game', out_dir=resultsFolder)
	xml.set('game', save='results.xml')

	#set optional variables
	for xset in xRun.root.findall('set'):
		path, attrib = xset.find('var').text.split('#')
		val = xset.find('val').text
		#print('set',path,attrib)
		if xml.get(path,attrib) is None:
			print ('xml set path not found :', path)
		xml.set(path, **{attrib : val})
	
	if not Args.executed:
		xml.save( runConfig.run_config )
		runConfig.method = runNormal
	else:
		runConfig.method = runFake
	return True, runConfig

def preparePlotRun(xRun, resultsFolder):
	use, runConfig = prepareRun(xRun, resultsFolder)
	if not use:
		return use, runConfig
	runConfig.x = readAxisDesc(xRun.find('xaxis'))
	runConfig.Y = [readAxisDesc(y) for y in xRun.findall('yaxis')]
	xs = xRun.find('save')
	runConfig.save = xs.text if xs is not None else None
	runConfig.title = xRun.find('name').text
	return use, runConfig

def doRun(runConfig, bindir):
	method = runConfig.method
	ret = method(runConfig.command_line, runConfig.run_name, bindir=bindir)
	return ret

def runNormal(command_line, run_name, bindir) :
	curdir = os.getcwd()
	os.chdir( bindir )
	print('executing : ',run_name)
	proc = subprocess.Popen("GameLauncher.exe " + command_line,stdout=subprocess.PIPE)
	pb = ProgressBar(100,prefix='progress', suffix='', decimals=0, length=50, fill='#')
	pb.print_progress_bar( 0 )
	for line in proc.stdout:
		try:
			pb.print_progress_bar( float(line.decode('utf-8')) )
		except ValueError:
			pass
	_, _ = proc.communicate() #fetch return code
	print('... Done! return code', proc.returncode)
	os.chdir( curdir )

def runFake(command_line, run_name, bindir) :
	pass

def readAxisDesc(xaxis):
	desc = AxisDescription()
	desc.label = xaxis.find('label').text
	desc.source = xaxis.find('source').text
	desc.var = xaxis.find('var').text
	xval = xaxis.find('val')
	if xval is not None:
		desc.values = readAxisValues(xval.text)
	xs = xaxis.find('save')
	desc.save = xs.text if xs is not None else None
	return desc

def makeDataRetrievalOperator(vartext):
	path,var = vartext.split('#')
	vs = var.split('(')
	if len(vs)==1:
		def readf(xml):
			return float( xml.get(path,var) )
		return readf
	else:
		op = vs[0]
		args = vs[1][:-1].split(',')
		if op == 'average':
			def avg_hist(xml):
				string = xml.get(path,args[0])
				#Vals = { float(e[0]) : float(e[1]) for e in [ e1.split(',') for e1 in string[:-1].split(';') ] }
				#ooTotSumWeights = 1.0 / sum( Vals.values )
				#Avg = sum( [v*ooTotSumWeights for v in Vals.keys] )
				sum_weights = 0
				weighted_sum = 0
				for e in string[:-1].split(';'):
					ee  = e.split(':')
					v = float(ee[0])
					w = float(ee[1])
					sum_weights += w
					weighted_sum += v*w
				return weighted_sum / sum_weights
			return avg_hist
		elif op=='select':
			def select_hist(xml):
				string = xml.get(path,args[0])
				sel = args[1]
				for e in string[:-1].split(';'):
					ee = e.split(':')
					if ee[0] == sel:
						return float(ee[1])
			return select_hist
		elif len(args)==2:
			opf = {'sum' : operator.add, 'div' : operator.truediv, 'sub' : operator.sub }[op]
			def binary_operator(xml):
				v1 = xml.get(path, args[0])
				v2 = xml.get(path, args[1])
				return opf(float(v1),float(v2))
			return binary_operator
		else:
			raise ValueError

def readAxisValues(string):
	r = string.split('..')
	if len(r) > 1:
		r1 = r[1].split(',')
		r = [r[0], r1[0]]
		r.append( r1[1] if len(r1)>1 else "1")
		try:
			_ = int(r[0])
			_ = int(r[1])
			if len(r) > 2: _ = int(r[2])
			conv = int
		except ValueError:
			conv = float
		rmin = conv(r[0])
		rmax = conv(r[1])
		step = conv(r[2]) if len(r) > 2 else conv(1)
		values = []
		while rmin <= rmax:
			values.append( str(rmin) )
			rmin += step
	else:
		try:
			values = [int(x) for x in string.split(',')]
		except ValueError:
			values = [float(x) for x in string.split(',')]
	return values

def readAxisData(desc, runs):
	Values = []
	get = makeDataRetrievalOperator(desc.var)
	for rc, ret in runs:
		src = rc.run_config if desc.source == 'run_config' else ret
		xml = XMLFile(src)
		try:
			xv = get(xml)
		except TypeError:
			print('readAxisData src is',src)
			print('axis',desc.label)
			raise
		#print('axis xval ',xv)
		Values.append( xv )
	return Values

def test():
	pass

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('-test', 	   action='store_true')
	parser.add_argument('config',    type=Path, help="test config file")
	parser.add_argument('-results',  type=Path, default='C:\\MyData\\Projects\\gra_w_pana\\results',help="result folder")
	parser.add_argument('-executed', type=str,  help='results folder name of already executed config file')
	parser.add_argument('-bin',		type=Path, default="c:\\MyData\\Projects\\gra_w_pana\\c++\\x64\\Release", help='bin directory')
	Args = parser.parse_args(sys.argv[1:])

	if Args.test:
		test()
	else:
		main(Args)
