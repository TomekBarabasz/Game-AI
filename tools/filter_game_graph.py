from pathlib import Path
from subprocess import run
import sys, argparse
from os import getcwd,remove

def filterEdgesBySrc(Edges, Src):
	return  [ (src,dst,line) for src,dst,line in Edges if src in Src ]

def filterEdgesByDst(Edges, Dst):
	return  [ (src,dst,line) for src,dst,line in Edges if dst in Dst ]

def loadGraph(filename):
	file = open(filename)

	Nodes = {}
	Edges = []

	for line in file:
		i = line.find('used=')
		if i > 0:
			j = line.find(" ")
			Nodes[ line[1:j-1] ] = ( line, line[i+6] )
		else:
			#edge format: "00000000002C9F28" -> "00000000002C99B0" [label
			i = line.find('" -> "')
			if i > 0:
				src = line[1 : i]
				j = line.find(" [label")
				dst = line[i+6 : j-1]
				Edges.append( (src,dst,line) )
	
	return Nodes, Edges

def filterGraph(Nodes, Edges, depth, root):
	filtered_nodes = set()
	filtered_edges = set()

	Ln_nodes = set([ k for k,v in Nodes.items() if v[1]=='1' ]) if root is None else root

	#discard used attribute
	Nodes = { k:v[0] for k,v in Nodes.items() } 

	while depth > 0:
		Ln_edges = set ( filterEdgesBySrc(Edges, Ln_nodes) )
		Ln_edges.update( filterEdgesByDst(Edges, Ln_nodes) )
		filtered_nodes.update( [Nodes[id] for id in Ln_nodes if id in Nodes] )
		filtered_edges.update( [ e[2] for e in Ln_edges ] )
		depth -= 1
		if depth > 0:
			Ln_nodes = set ( [dst for src,dst,line in Ln_edges] )
			Ln_nodes.update( [src for src,dst,line in Ln_edges] )
	
	return filtered_nodes, filtered_edges

def writeGrapFile(nodes, edges, filename):
	file = open(filename,'w')
	file.write( 'digraph g {\n')
	for node in nodes:
		file.write(node)
	for edge in edges:
		file.write(edge)
	file.write( '}\n' )

def main(Args):
	Nodes, Edges = loadGraph(Args.file)
	nodes, edges = filterGraph(Nodes, Edges, Args.depth, Args.root)
	writeGrapFile(nodes, edges, Args.out)

def findDuplicates(filename):
	file = open(filename)
	Nodes = set()
	for line in file:
		i = line.find('used=')
		if i > 0:
			label = line[29:97]
			if label in Nodes:
				id = line[1:17]
				print('Found duplicate node ',id)
				break
			else:
				Nodes.add(label)

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('file',			type=Path, help="file to convert")
	parser.add_argument('-out',			type=Path, help="output filename")
	parser.add_argument('-exe',  '-e',	type=str, default='dot.exe', help="graph creator executable like dot.exe or twopi.exe")
	parser.add_argument('-depth','-d',	type=int, default=1, help="depth from game path")
	parser.add_argument('-root', '-r',	type=str, help="root node")
	parser.add_argument('-findduplicates',	action='store_true', help="root node")
	Args = parser.parse_args(sys.argv[1:])
	if Args.findduplicates:
		findDuplicates(Args.file)
	else:
		main(Args)
