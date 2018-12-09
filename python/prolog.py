from ctypes import *
from os import chdir
from itertools import count
import sys

class SwiProlog:
	def __init__(self, **kwargs):
		dllFileName = kwargs.get('dll',"C:\\Program Files\\swipl\\bin\\libswipl.dll")
		dir = dllFileName[0 : dllFileName.rfind('\\')]
		chdir(dir)
		self.dll = cdll.LoadLibrary(dllFileName)
		self.dll.PL_initialise.restype = c_int
		LP_LP_char = POINTER(POINTER(c_char))
		self.dll.PL_initialise.argtypes = [c_int, LP_LP_char]
		self.argv = (POINTER(c_char) * 3)()
		self.argv[0] = create_string_buffer("".encode())
		#self.argv[0] = create_string_buffer(dllFileName.encode())
		#self.argv[1] = create_string_buffer("-x".encode())
		#self.argv[2] = create_string_buffer("mystate".encode())
		result = self.dll.PL_initialise(1, self.argv)
		assert(result==1)
		print('swipl initialized')
	
	def __del__(self):
		self.dll.PL_toplevel.restype = c_int
		self.dll.PL_toplevel.argtypes = None
		tl = self.dll.PL_toplevel()
		self.dll.PL_halt.restype = c_int
		self.dll.PL_halt.argtypes = [c_int]
		result = self.dll.PL_halt(tl)
		assert(result==1)

class Term:
	YAP_Term    = c_void_p
	YAP_Atom    = c_void_p
	YAP_Functor = c_void_p
	dll = None
	
	@classmethod
	def init(cls,dll):
		cls.dll = dll
		
		cls.dll.YAP_LookupAtom.restype = cls.YAP_Atom
		cls.dll.YAP_LookupAtom.argtypes = [c_char_p]
		
		cls.dll.YAP_MkAtomTerm.argtypes = [cls.YAP_Atom]
		cls.dll.YAP_MkAtomTerm.restype = cls.YAP_Term
		
		cls.dll.YAP_MkFunctor.restype = cls.YAP_Functor
		cls.dll.YAP_MkFunctor.argtypes = [cls.YAP_Atom, c_longlong]
		
		cls.dll.YAP_MkApplTerm.restype = cls.YAP_Term
		cls.dll.YAP_MkApplTerm.argtypes = [cls.YAP_Functor, c_longlong, POINTER(cls.YAP_Term)]
		
		cls.dll.YAP_AtomName.argtypes = [cls.YAP_Term]
		cls.dll.YAP_AtomName.restype = c_char_p
		
		cls.dll.YAP_AtomOfTerm.restype = cls.YAP_Atom
		cls.dll.YAP_AtomOfTerm.argtypes = [cls.YAP_Term]
		
		cls.dll.YAP_MkIntTerm.restype = cls.YAP_Term
		cls.dll.YAP_MkIntTerm.argtypes = [c_longlong]
		
		cls.dll.YAP_IntOfTerm.restype = c_longlong
		cls.dll.YAP_IntOfTerm.argtypes = [cls.YAP_Term]
	
		cls.dll.YAP_MkFloatTerm.restype = cls.YAP_Term
		cls.dll.YAP_MkFloatTerm.argtypes = [c_double]
	
		cls.dll.YAP_FloatOfTerm.restype = c_double
		cls.dll.YAP_FloatOfTerm.argtypes = [cls.YAP_Term]

		cls.dll.YAP_MkVarTerm.argtypes = None
		cls.dll.YAP_MkVarTerm.restype = cls.YAP_Term
		
		cls.dll.YAP_IsVarTerm.restype = c_int
		cls.dll.YAP_IsVarTerm.argtypes = [cls.YAP_Term]
		cls.dll.YAP_IsNonVarTerm.restype = c_int
		cls.dll.YAP_IsNonVarTerm.argtypes = [cls.YAP_Term]
		cls.dll.YAP_IsAtomTerm.restype = c_int
		cls.dll.YAP_IsAtomTerm.argtypes = [cls.YAP_Term]
		
		cls.dll.YAP_IsIntTerm.restype = c_int
		cls.dll.YAP_IsIntTerm.argtypes = [cls.YAP_Term]
		cls.dll.YAP_IsFloatTerm.restype = c_int
		cls.dll.YAP_IsFloatTerm.argtypes = [cls.YAP_Term]
		cls.dll.YAP_IsPairTerm.restype = c_int
		cls.dll.YAP_IsPairTerm.argtypes = [cls.YAP_Term]
		cls.dll.YAP_IsApplTerm.restype = c_int
		cls.dll.YAP_IsApplTerm.argtypes = [cls.YAP_Term]
		
		cls.dll.YAP_ArgOfTerm.restype = cls.YAP_Term
		cls.dll.YAP_ArgOfTerm.argtypes = [c_int, cls.YAP_Term]
	
	def isvar(self):
		return Term.dll.YAP_IsVarTerm(self.t)
	def isatom(self):
		return Term.dll.YAP_IsAtomTerm(self.t)
	def getTerm(self):
		return self.t
	def __str__(self):
		return self.value()

class AtomTerm(Term):
	def __init__(self, name):
		self.sb = create_string_buffer(name.encode())
		a = Term.dll.YAP_LookupAtom(self.sb)
		self.t = Term.dll.YAP_MkAtomTerm( a )
	def value(self):
		a = Term.dll.AtomOfTerm(self.t)
		return Term.atomName(a)

class IntTerm(Term):
	def __init__(self, value):
		self.t = Term.dll.YAP_MkIntTerm(value)
	def value(self):
		return Term.dll.YAP_IntOfTerm(self.t)

class FloatTerm(Term):
	def __init__(self, value):
		self.t = Term.dll.YAP_MkFloatTerm(value)
	def value(self):
		return Term.dll.YAP_FloatOfTerm(self.t)

class VariableTerm(Term):
	def __init__(self):
		self.t = Term.dll.YAP_MkVarTerm()
	def value(self):
		return None

class CompoundTerm:
	def __init__(self, functor, args):
		self.args = args
		arity = len(args)
		self.sb = create_string_buffer(functor.encode())
		a = Term.dll.YAP_LookupAtom(self.sb)
		f = Term.dll.YAP_MkFunctor(a, arity)
		Args = (Term.YAP_Term * arity)()
		for ar,i in zip(args, count(0)):
			Args[i] = ar.getTerm()
		self.t = Term.dll.YAP_MkApplTerm(f, len(args), Args)
	def getTerm(self):
		return self.t
	def __getitem__(self, idx):
		t = Term.dll.YAP_ArgOfTerm(idx, self.t)
		if Term.dll.YAP_IsAtomTerm(t):
			return Term.dll.YAP_AtomName(Term.dll.YAP_AtomOfTerm(t)).decode()
		elif Term.dll.YAP_IsIntTerm(t):
			return Term.dll.YAP_IntOfTerm(t)
		elif Term.dll.YAP_IsFloatTerm(t):
			return Term.dll.YAP_FloatOfTerm(t)
		elif Term.dll.YAP_IsPairTerm(t):
			return 'pair'
		elif Term.dll.YAP_IsApplTerm(t):
			return 'compound'
	
class YAProlog:
	def __init__(self, **kwargs):
		dllFileName = kwargs.get('dll',"C:\\Program Files\\Yap64\\bin\\yap.dll")
		dir = dllFileName[0 : dllFileName.rfind('\\')]
		chdir(dir)
		self.dll = cdll.LoadLibrary(dllFileName)
		self.dll.YAP_FastInit.restype = c_int
		self.dll.YAP_FastInit.argtypes = [c_char_p]
		result = self.dll.YAP_FastInit(c_char_p(0))
		assert(result != -1)
		self.dll.YAP_RunGoalOnce.restype = c_int
		self.dll.YAP_RunGoalOnce.argtypes = [Term.YAP_Term]
		self.dll.YAP_RunGoal.restype = c_longlong
		self.dll.YAP_RunGoal.argtypes = [Term.YAP_Term]
		self.dll.YAP_RestartGoal.restype = c_longlong
		self.dll.YAP_RestartGoal.argtypes = None
		Term.init(self.dll)
		print('yap initialized')
		if 'file' in kwargs:
			file = kwargs['file']
			#print('creating compile goal for file',file)
			t = CompoundTerm('compile', [ AtomTerm(file) ])
			#print('running compile goal')
			result = self.runGoalOnce(t)
			assert(result == 1)
		
	def runGoalOnce(self,goal):
		res = self.dll.YAP_RunGoalOnce(goal.getTerm())
		return res
	
	def runGoal(self,goal):
		res = self.dll.YAP_RunGoal(goal.getTerm())
		return res
	
	def restartGoal(self):
		res = self.dll.YAP_RestartGoal()
		return res
	
	def runForAllSolutons(self, goal):
		solutions=[]
		Vidx = [ i for arg,i in zip(goal.args,count(1)) if arg.isvar() ]
		print(Vidx)
		if self.dll.YAP_RunGoal(goal.getTerm()):
			while True:
				sln = [ goal[idx] for idx in Vidx ]
				solutions.append(sln)
				if 0 == self.restartGoal():
					break
		return solutions
	
	def __del__(self):
		self.dll.YAP_Exit.restype = None
		self.dll.YAP_Exit.argtypes = [c_int]
		self.dll.YAP_Exit(0)
	
	def test1(self):
		#res = self.runGoalOnce( CompoundTerm( 'writeln', [ AtomTerm('dupa') ] ) )
		#print('runGoalOnce done, result',res)
		goal = CompoundTerm('woman', [VariableTerm()])
		if self.runGoal(goal):
			while True:
				print('variable value is',goal[1])
				if 0 == self.restartGoal():
					break
	def test2(self):
		print('test2')
		goal = CompoundTerm('woman', [VariableTerm()])
		Solutions = self.runForAllSolutons(goal)
		print(Solutions)
	
	def test3(self):
		print('test3')
		goal = CompoundTerm('path',[IntTerm(1), IntTerm(2), VariableTerm()])
		Solutions = self.runForAllSolutons(goal)
		print(Solutions)

def testSWI():
	e = SwiProlog()

def testYAP(file=None):
	e = YAProlog(**{'file':file} if file is not None else {})
	e.test2()
	e.test3()

if __name__ == '__main__':
	testYAP(sys.argv[1] if len(sys.argv) >=2 else None)
