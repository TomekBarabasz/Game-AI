def python36():
	i=10
	j=1_00
	print( f"to jest i {i} to jest j {j:_} to jest k {0xFF_FF}")

@dataclass
class Point:
	x:float
	y:float
	z:float=0

def python37():
	p = Point(1.2, 2.3)
	print(p)
	pass

def python38():
	pass

python36()
python37()
python38()
