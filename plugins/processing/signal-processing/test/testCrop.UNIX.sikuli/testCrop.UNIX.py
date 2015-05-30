def setUp(self):
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " +
             ov_binany_path +"/openvibe-designer.sh --no-session-management --open " +
            ov_binany_path +"/share/openvibe/scenarios/box-tutorials/crop.xml")
    while not self.terminal.window():
        wait(1)
    wait("play.png",10)
    
def testRunCrop(self):
    click("play.png")
    wait("timesignal.png",10)
def tearDown(self):
    mouseMove(Location(0,0))
    if self.terminal.window():
        App.close(self.terminal)
        self.terminal= None

