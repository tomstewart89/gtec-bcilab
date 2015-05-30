import sys
import subprocess
import time

#this script is to be called by the dart test script to launch the openvibe scenario used to replay p300 experiment data
#and then the external app

class P300Test: 

	#launch the external stimulator
	def startExternalStimulator(self):   
		exprocess = subprocess.Popen([self.configPath+'/'+'ov-externalP300Stimulator'+self.extension])
		return exprocess
	#launch a scenario
	def startScenario(self, scenario, fast): 
		print self.configPath
		print self.Path_Samples
		print scenario
		if fast==True:
		    ovprocess = subprocess.Popen([self.configPath+'/'+'openvibe-designer'+self.extension,'--no-gui','--play-fast',self.Path_Samples+'/'+scenario])
		else:
		    ovprocess = subprocess.Popen([self.configPath+'/'+'openvibe-designer'+self.extension,'--no-gui','--play',self.Path_Samples+'/'+scenario]) 
		    
		return ovprocess

	def __init__(self):
		#path to dist
		self.configPath = sys.argv[1]
		#path to the p300 test folder
		self.Path_Samples = self.configPath+"../contrib/applications/platform/P300Stimulator/test"
		platform = sys.platform
		if platform[0:3]=='lin':
		    self.extension = '.sh'
		elif platform[0:3]=='win':
		    self.extension = '.cmd'
		else:
		    self.extension = '.sh'

if __name__ == "__main__":
	Test = P300Test()
	#launch openvibe first to have the shared memory
	ovprocess = Test.startScenario("p300-speller-replay.xml", False)
	#if we do not wait long enough(5 sec maybe less but more than one), the shared memory seem filled with random stuff and the external app behaves weirdly
	#changing this value will change the timings and make the tests fails
	time.sleep(2)#14.152
	#launch external app
	exprocess = Test.startExternalStimulator()
	exprocess.wait()
	ovprocess.wait()
