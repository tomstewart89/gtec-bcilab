def setUp(self):
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " + ov_binany_path +"/openvibe-designer.sh --no-session-management --play testOgreContext.xml")
    while not self.terminal.window():
        wait(1)
    wait("Simple3Dview.png",10)
def testRunOgreVisual(self):
    waitVanish("Simple3Dview.png",10)
    assert(exists("designerScreen.png"))
def tearDown(self):
    if self.terminal.window():
        App.close(self.terminal)
        self.terminal= None

