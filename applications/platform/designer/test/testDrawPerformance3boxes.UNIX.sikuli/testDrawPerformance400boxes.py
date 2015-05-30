

def setUp(self):
    file="Serial400boxTest.xml"
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " 
            + ov_binany_path 
            +"/openvibe-designer.sh --no-session-management --open "+ file)
    while not self.terminal.window():
        wait(1)
    wait("SinusOscillatorChained.png",10)
def test_drawBoxes(self):
    import time
    
    start=time.time()
    click(Pattern("SinusOscillatorChained.png").targetOffset(51,-37))
   
    while not exists("Signaldisplay400Boxes.png"):
        wheel( WHEEL_DOWN , 20)
    print "ok scroll up"
    while not exists(Pattern("SinusOscillatorChained.png").similar(0.75)):
        wheel( WHEEL_UP , 20)

    print "ok scroll down"
    while not exists("Signaldisplay400Boxes.png"):
        wheel( WHEEL_DOWN , 20)
    print "ok scroll up"
    while not exists(Pattern("SinusOscillatorChained.png").similar(0.75)):
        wheel( WHEEL_UP , 20)
    print "ok scroll down"
    stop=time.time()
    print "time to scroll= "+ str(stop-start)
   
def tearDown(self):
    App.close(self.terminal)
    self.terminal= None

