def setUp(self):
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " + ov_binany_path +"/openvibe-designer.sh --no-session-management")
    while not self.terminal.window():
        wait(1)
    wait("StartInterface.png",10)
def test_createSimpleScenarioAndRun(self):
    click(Pattern("Datagenerati.png").targetOffset(-70,-1))
    dragDrop("Sinusoscilla-1.png",Pattern("DesignerDataGenOpen.png").targetOffset(-233,-163))
    assert(exists("SinusOscillatorBoxSelected.png"))
    click(Pattern("Visualisatio.png").targetOffset(-62,0))
    click(Pattern("Basic.png").targetOffset(-50,-1))
    dragDrop("Signaldisplay.png", Pattern("designerSinusGenPlaced.png").targetOffset(-197,-16))
    assert(exists("SignalDisplayBoxSelected.png"))
    dragDrop("outputSignalConnector.png", "imputSingnalConnector.png")
    click(Pattern("playButton.png").similar(0.95))
    wait(6)
    assert(exists(Pattern("SignalDisplayWindow.png").similar(0.46)))
    dragDrop(Pattern("SignalDisplayWindow.png").similar(0.46).targetOffset(2,-157),Pattern("SignalDisplayWindow.png").similar(0.46).targetOffset(-102,137))
    click("stopButton.png")
    waitVanish(Pattern("SignalDisplayWindow.png").similar(0.58))
    type("w",KeyModifier.CTRL)
    assert(exists("SavechangePopup.png"))
    click(Pattern("SavechangePopup.png").targetOffset(-170,56))
def test_boxSetAtributes(self):
    click("SearchBoxBar.png")
    paste("sinus")
    dragDrop("Sinusoscilla-1.png",Pattern("DesignerDataGenOpen.png").similar(0.40).targetOffset(-233,-163))
    rightClick("SinusOscillatorBoxSelected.png")
    click(Pattern("contextualBoxMenu.png").targetOffset(-51,16))
    assert(exists("renameBoxPopUp.png"))
    type("XXXX XXXX XXXX"+ Key.ENTER)
    assert(exists("SinusOscillatorNewNameXXX.png"))
    
def tearDown(self):
    App.close(self.terminal)
    self.terminal= None

