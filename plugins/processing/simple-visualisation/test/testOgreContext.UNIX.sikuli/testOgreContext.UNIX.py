def setUp(self):
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " + ov_binany_path +"/openvibe-designer.sh --no-session-management --open testOgreContext.xml")
    while not self.terminal.window():
        wait(1)
    wait("designerScreen.png",10)
def testRunOgreVisual(self):
    click("play.png")
    mouseMove(Location(0,0))
    
    wait("Simple3Dview.png",10)
    waitVanish("Simple3Dview.png",10)
    assert(exists("designerScreen.png"))
def tearDown(self):
    mouseMove(Location(0,0))
    if self.terminal.window():
        App.close(self.terminal)
        self.terminal= None

