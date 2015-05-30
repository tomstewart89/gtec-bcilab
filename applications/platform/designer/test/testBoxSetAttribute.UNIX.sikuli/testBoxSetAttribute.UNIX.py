def setUp(self):
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " + ov_binany_path +"/openvibe-designer.sh --no-session-management")
    while not self.terminal.window():
        wait(1)
    wait("StartInterface.png",100)

def test_boxSetAttributes(self):
    click("SearchBoxBar.png")
    paste("sinus")
    wait(3)
    dragDrop("Sinusoscilla-1.png",Pattern("DesignerDataGenOpen.png").similar(0.40).targetOffset(-233,-163))
    rightClick("SinusOscillatorBoxSelected.png")
    click(Pattern("contextualBoxMenu.png").targetOffset(-51,16))
    assert(exists("renameBoxPopUp.png"))
    type("XXXX XXXX XXXX"+ Key.ENTER)
    assert(exists(Pattern("SinusOscillatorNewNameXXX.png").similar(0.50)))
    
def tearDown(self):
    mouseMove(Location(0,0))
    App.close(self.terminal)
    self.terminal= None
    wait(2)

