def setUp(self):
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " + ov_binany_path +"/openvibe-designer.sh --no-session-management")
    while not self.terminal.window():
        wait(1)
    wait("StartInterface.png",10)
def test_createSimpleScenarioAndRun(self):
    click(Pattern("Datagenerati.png").targetOffset(-70,-1))
    wait(3)
    dragDrop("Sinusoscilla-1.png",Pattern("DesignerDataGenOpen.png").targetOffset(-233,-163))
    assert(exists("SinusOscillatorBoxSelected.png"))
    click(Pattern("Visualisatio.png").targetOffset(-62,0))
    click(Pattern("Basic.png").targetOffset(-50,-1))
    wait(3)
    dragDrop("Signaldisplay.png", Pattern("designerSinusGenPlaced.png").similar(0.50).targetOffset(-197,-16))
    assert(exists("SignalDisplayBoxSelected.png"))
    dragDrop("outputSignalConnector.png", "imputSingnalConnector.png")
    click(Pattern("playButton.png").similar(0.95))
    wait(6)
    assert(exists(Pattern("SignalDisplayWindow.png").similar(0.46),100))
    dragDrop(Pattern("SignalDisplayWindow.png").similar(0.46).targetOffset(-18,-168),Pattern("SignalDisplayWindow.png").similar(0.46).targetOffset(-102,137))
    click("stopButton.png")
    waitVanish(Pattern("SignalDisplayWindow.png").similar(0.46))
    type("w",KeyModifier.CTRL)
    assert(exists("SavechangePopup.png",3))
    click(Pattern("SavechangePopup.png").targetOffset(-170,56))
    waitVanish(Pattern("SavechangePopup.png").targetOffset(-170,56),3)
    type("n",KeyModifier.CTRL)
    wait(1)
    
    
def tearDown(self):
    mouseMove(Location(0,0))
    if self.terminal.window():
        App.close(self.terminal)
        self.terminal= None

