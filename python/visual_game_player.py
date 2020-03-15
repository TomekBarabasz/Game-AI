import sys, socket, argparse, cocos, aiohttp, asyncio

async def server_handle_event(reader,writer):
	data = reader.read(512)
	message = data.decode()
	addr = writer.get_extra_info('peername')
	print('recv {0} from {1}'.format(message, addr))
	writer.write(data)
	await writer.drain()
	writer.close()

def server_main(Args):
	loop = asyncio.get_event_loop()
	coro = asyncio.start_server(server_handle_event, Args.addr, Args.port, loop=loop)
	server = loop.run_until_complete(coro)
	print('serving on {0}'.format( server.sockets[0].getsockname()) )
	try:
		loop.run_forever()
	except KeyboardInterrupt:
		pass
	server.close()
	loop.run_until_complete(server.wait_closed())
	loop.close()

def client_main(Args):
	reader,writer = asyncio.open_connection(Args.addr, Args.port)#, loop=loop)



if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('-addr', 	   type=str, default='127.0.0.1', help='ip address for connection[s]')
	parser.add_argument('-port', 	   type=int, default=50007, help='ip port for connection[s]')
	parser.add_argument('-game',     type=str, help="game name")
	parser.add_argument('-server',   action='store_true', help="starts server if specified")
	Args = parser.parse_args(sys.argv[1:])
	if Args.server:
		server_main(Args)
	else:
		client_main(Args)
	
