import asyncio, sys, aiohttp, signal
from datetime import datetime
from threading import Thread
from time import sleep

async def say(what,when):
	await asyncio.sleep(when)
	print('time {0} say {1}'.format(datetime.now(), what) )

async def stop(loop,when):
	await asyncio.sleep(when)
	print('time {0} stop'.format(datetime.now()) )
	loop.stop()

def test1():
	print('test1 start',datetime.now())
	loop = asyncio.get_event_loop()
	loop.run_until_complete(say('hello world',1))
	loop.close()

def test2():
	print('test2 start',datetime.now())
	loop = asyncio.get_event_loop()
	loop.create_task( say('first hello',2))
	loop.create_task( say('second hello',1))
	loop.create_task( stop(loop,3))
	try:
		loop.run_forever()
	except KeyboardInterrupt:
		pass
	loop.close()

def test3():
	print('test3 start',datetime.now())
	loop = asyncio.get_event_loop()
	loop.run_until_complete(
		asyncio.gather(
			say('first hello',2),
			say('second hello',1)
		)
	)
	loop.close()

async def test4_main():
	print('test4 start',datetime.now())
	t1 = asyncio.create_task( say('hello 1',1))
	t2 = asyncio.create_task( say('hello 2',2))
	await t1
	await t2

def test4():
	asyncio.run( test4_main() )

async def fetch_page(session,url):
	#with aiohttp.Timeout(10):
		async with session.get(url) as response:
			assert response.status == 200
			return await response.text()

async def test5_main():
	async with aiohttp.ClientSession() as session:
		content = await fetch_page(session,'http://python.org')
		print(content)

def test5():
	print('test5 start',datetime.now())
	loop = asyncio.get_event_loop()
	loop.run_until_complete(test5_main())
	loop.close()

async def cycling():
	i=1
	while True:
		print(i)
		i+=1
		await asyncio.sleep(1)

g_task = None
g_loop = None
g_server = None
g_queue = None

def thread_main():
	global g_task
	global g_loop
	print('thread start',datetime.now())
	asyncio.set_event_loop(asyncio.new_event_loop())
	g_loop = asyncio.get_event_loop()
	g_task = g_loop.create_task(cycling())
	g_loop.run_forever()
	print('thread stop',datetime.now())
	g_loop.close()

def test6():
	print('test6 start',datetime.now())
	t = Thread(target=thread_main)
	t.start()
	sleep(3)
	print('test6 cancel',datetime.now())
	g_task.cancel()
	g_loop.stop()

async def client_connected(reader,writer):
	data = reader.read(512)
	message = data.decode()
	addr = writer.get_extra_info('peername')
	print(f'recv {message} from {addr}')
	writer.write(data)
	await writer.drain()
	writer.close()

def server_thread_main():
	global g_task
	global g_loop
	global g_server
	print('thread start',datetime.now())
	asyncio.set_event_loop(asyncio.new_event_loop())
	g_loop = asyncio.get_event_loop()
	start = asyncio.start_server(client_connected, '127.0.0.1', 50007, loop=g_loop)
	server = g_server = g_loop.run_until_complete(start)
	print(f'serving on {server.sockets[0].getsockname()}')
	g_task = g_loop.create_task(server.serve_forever())
	try:
		#both will work
		g_loop.run_forever()
		#g_loop.run_until_complete(g_task)
	except asyncio.CancelledError:
		pass
	print('thread stop',datetime.now())
	g_loop.close()

def test7():
	print('test7 start',datetime.now())
	#note nie działa z server_thread_main_2 ?? dlaczego ??
	t = Thread(target=server_thread_main)
	t.start()
	sleep(3)
	print('test7 cancel',datetime.now())
	#server_thread_main needs all three
	if g_task: g_task.cancel()
	if g_loop: g_loop.stop()
	if g_server: g_server.close()

async def server_main():
	global g_task
	global g_server
	print('server main start',datetime.now())
	srv = g_server = await asyncio.start_server(client_connected, '127.0.0.1', 50007)
	print(f'serving on {srv.sockets[0].getsockname()}')
	await srv.serve_forever()
	
def server_thread_main_2():
	print('thread start',datetime.now())
	asyncio.run(server_main())
	print('thread stop',datetime.now())

def test71():
	#note nie działa z server_thread_main_2 ?? dlaczego ??
	print('test71 start',datetime.now())
	t = Thread(target=server_thread_main_2)
	t.start()
	sleep(3)
	print('test7 cancel',datetime.now())
	#server_thread_main needs all three
	if g_task: g_task.cancel()
	if g_loop: g_loop.stop()
	if g_server: g_server.close()

async def print_from_queue(queue):
	while True:
		#msg = await queue.get()
		get = asyncio.create_task(queue.get())
		done,pending = await asyncio.wait( {get} )
		msg = await get
		print('received',msg)
		if msg=='exit':
			break

async def test8_amain():
	global g_task
	global g_loop
	global g_queue
	print('async main start',datetime.now())
	g_queue = asyncio.Queue()
	#note: zakomentowane linie są potrzebne jak by to było zwykłe main (thread_main)
	#w async main to jest zbędne i błędne

	#asyncio.set_event_loop(asyncio.new_event_loop())
	#g_loop = asyncio.get_event_loop()
	#g_loop.run_until_complete(print_from_queue(g_queue))
	g_queue.put_nowait('init')
	await print_from_queue(g_queue)
	print('async main stop',datetime.now())
	#g_loop.close()

def test_main(amain):
	print('thread start',datetime.now())
	asyncio.run(amain())
	print('thread stop',datetime.now())

def test8():
	thread = Thread(target=test_main,args=(test8_amain,))
	thread.start()
	sleep(1)
	print('put_nowait msg 1, queue size is',g_queue.qsize())
	g_queue.put_nowait('msg 1')
	g_queue._loop._write_to_self()
	sleep(1)
	print('put_nowait msg 2, queue size is',g_queue.qsize())
	g_queue.put_nowait('msg 2')
	g_queue._loop._write_to_self()
	sleep(1)
	print('put_nowait exit, queue size is',g_queue.qsize())
	g_queue.put_nowait('exit')
	g_queue._loop._write_to_self()

async def print_from_queue_2(queue):
	while True:
		msg = await queue.get()
		print('received',msg)

async def shutdown(loop):
	tasks = [t for t in asyncio.all_tasks() if t is not asyncio.current_task()]
	[task.cancel() for task in tasks]
	await asyncio.gather(*tasks, return_exceptions=True)
	loop.stop()

def test9_main():
	global g_queue
	print('async main start',datetime.now())
	g_queue = asyncio.Queue()
	asyncio.set_event_loop(asyncio.new_event_loop())
	loop = asyncio.get_event_loop()
	loop.add_signal_handler(signal.SIGTERM, shutdown(loop))
	loop.run_until_complete(print_from_queue_2(g_queue))
	print('async main stop',datetime.now())
	loop.close()

def test9():
	thread = Thread(target=test9_main)
	thread.start()
	sleep(1)
	print('put_nowait msg 1, queue size is',g_queue.qsize())
	g_queue.put_nowait('msg 1')
	g_queue._loop._write_to_self()
	sleep(1)
	print('put_nowait msg 2, queue size is',g_queue.qsize())
	g_queue.put_nowait('msg 2')
	g_queue._loop._write_to_self()
	sleep(1)
	print('sending SIGTERM')
	

if __name__ == '__main__':
	{	't1' : test1,
		't2' : test2,
		't3' : test3,
		't4' : test4,
		't5' : test5,
		't6' : test6,
		't7' : test7,
		't71': test71,
		't8' : test8,
		't9' : test9 }[sys.argv[1]]()

