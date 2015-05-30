
class MyOVBox(OVBox):
	def __init__(self):
		OVBox.__init__(self)
		
	def initialize(self):
		# nop
		return
		
	def process(self):
		# print the string specified in the box configuration. 
		# 'Message' is the name of the config entry.
		print(self.setting['Message']) 
		
	def uninitialize(self):
		# nop
		return

box = MyOVBox()
