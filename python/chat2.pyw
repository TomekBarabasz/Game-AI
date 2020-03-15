import wx, asyncio, signal
from threading import Thread
from datetime import datetime
from time import sleep

g_task=None
g_loop=None
g_server=None
g_writers=[]
g_queue = None
g_frame = None

def forward(writer,addr,message):
	for w in g_writers:
		if w != writer:
			w.write( f"{addr!r}:{message!r}\n".encode() )

async def client_connected(reader,writer):
	global g_writers
	g_writers.append( writer )
	addr = writer.get_extra_info('peername')
	message = "connected"
	print(f'{addr!r}{message}')
	forward(writer,addr,message)
	while True:
		try:
			data = await reader.readline()
		except ConnectionResetError:
			forward(writer,addr,'exit')
			break
		message = data.decode().strip()
		forward(writer,addr,message)
		#print(f'server got {addr!r}{message!r}')
		await writer.drain()
		if message=='exit':
			break
	print(f'{addr!r}disconnected')
	g_writers.remove(writer)
	writer.close()

def server_main(addr,port):
	global g_task
	global g_loop
	global g_server
	print('server start',datetime.now())
	asyncio.set_event_loop(asyncio.new_event_loop())
	g_loop = asyncio.get_event_loop()
	start = asyncio.start_server(client_connected, addr, port, loop=g_loop)
	server = g_server = g_loop.run_until_complete(start)
	print(f'serving on {server.sockets[0].getsockname()}')
	g_task = g_loop.create_task(server.serve_forever())
	try:
		g_loop.run_forever()
	except asyncio.CancelledError:
		pass
	print('server stop',datetime.now())
	g_loop.close()
	g_task = g_loop = g_server = None

async def client_main_a(addr,port):
	global g_queue
	print('trying to connect')
	reader,writer = await asyncio.open_connection(addr,port)
	print('connected to server')
	g_queue = asyncio.Queue()
	remote = asyncio.create_task(reader.readline())
	local = asyncio.create_task(g_queue.get())
	while True:
		done,pending = await asyncio.wait( {remote, local}, return_when=asyncio.FIRST_COMPLETED)
		if remote in done:
			remote_msg = await remote
			if len(remote_msg)>0:
				#print('got remote msg')
				remote_msg = remote_msg.decode().strip()
				if g_frame:
					g_frame.onNewRemoteLine(remote_msg)
			remote = asyncio.create_task(reader.readline())
		if local in done:
			#print('got local msg')
			local_msg = await local
			writer.write( (local_msg+'\n').encode() )
			await writer.drain()
			if local_msg == 'exit':
				break
			else:
				local = asyncio.create_task(g_queue.get())
		"""do_exit = False
		for f in asyncio.as_completed({remote,local}):
			res = await f
			print(f'got {res} from {f}')
			if res == 'exit':
				do_exit=True
			if f == remote:
				print('got remote msg')
				if g_frame:
					g_frame.onNewRemoteLine(res.decode().strip())
			elif f == local:
				print('got local msg')
				writer.write( res.encode() )
				await writer.drain()
				if res == 'exit':
					do_exit=True
		if do_exit:
			break"""
	print('disconnecting from server')
	writer.close()
	await writer.wait_closed()

def client_main(addr,port):
	asyncio.run(client_main_a(addr,port))

def put_nowait(queue,msg):
	queue.put_nowait(msg)
	queue._loop._write_to_self()

class MyFrame(wx.Frame):
	def __init__(self):
		super().__init__(None, title= "Chat", size=(500,500))	
		self.CreateStatusBar()
		self._createMenubar()
		self._createTextFields()
		self.server_thread = None
		self.client_thread = None
		self.Show(True)
	def _createMenubar(self):
		filemenu = wx.Menu()
		self.startServer = filemenu.AppendCheckItem(wx.ID_OPEN, '&Start chat server', 'Start/Stop chat server')
		self.Bind( wx.EVT_MENU, self.onStartSever, self.startServer )
		self.connect = filemenu.AppendCheckItem(wx.ID_CLOSE, '&Connect to chat server', 'Connect/disconnect chat server')
		self.Bind( wx.EVT_MENU, self.onConnect, self.connect )
		filemenu.AppendSeparator()
		self.Bind( wx.EVT_MENU, self.onExit, filemenu.Append(wx.ID_EXIT, 'E&xit', 'Terminate the program'))
		menuBar = wx.MenuBar()
		menuBar.Append(filemenu, '&File')
		self.SetMenuBar(menuBar)
	def _createTextFields(self):
		self.read = wx.TextCtrl(self, style=wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_RICH2)
		self.write = wx.TextCtrl(self)
		self.Bind(wx.EVT_TEXT_ENTER, self.onNewLocalLine, self.write)
		self.sizer = wx.BoxSizer(wx.VERTICAL)
		self.sizer.Add(self.read,  3, wx.EXPAND)
		self.sizer.Add(self.write, 0, wx.EXPAND)
		self.SetSizer(self.sizer)
		self.SetAutoLayout(1)
		#self.sizer.Fit(self)
	def onStartSever(self, event):
		global g_task
		global g_loop
		global g_server
		if self.startServer.IsChecked():
			self.startServer.SetItemLabel("&Stop chat server")
			self.server_thread = Thread(target=server_main, args=('127.0.0.1',50007))
			self.server_thread.start()
		else:
			self.startServer.SetItemLabel("&Start chat server")
			if g_task: g_task.cancel()
			if g_loop: g_loop.stop()
			if g_server: g_server.close()
	def onConnect(self,event):
		global g_queue
		if self.connect.IsChecked():
			self.connect.SetItemLabel("&Disconnect from chat server")
			self.client_thread = Thread(target=client_main, args=('127.0.0.1',50007))
			self.client_thread.start()
		else:
			self.connect.SetItemLabel("&Connect to chat server")
			put_nowait(g_queue,'exit')
	def onExit(self, event):
		global g_task
		global g_loop
		global g_server
		global g_queue
		if g_queue: 
			put_nowait(g_queue,'exit')
			if self.client_thread : self.client_thread.join()
		if g_task: g_task.cancel()
		if g_loop: g_loop.stop()
		if g_server: g_server.close()
		if self.server_thread : self.server_thread.join()
		self.Close(True)
	def onNewLocalLine(self, event):
		self.read.SetDefaultStyle(wx.TextAttr(wx.RED, alignment=wx.TEXT_ALIGNMENT_RIGHT))
		self.read.AppendText(event.String+'\n')
		self.write.SetValue('')
		if g_queue: put_nowait(g_queue,event.String)
	def onNewRemoteLine(self,txt):
		self.read.SetDefaultStyle(wx.TextAttr(wx.BLUE, alignment=wx.TEXT_ALIGNMENT_LEFT))
		self.read.AppendText(txt+'\n')

app = wx.App(False)
g_frame = MyFrame()
app.MainLoop()
