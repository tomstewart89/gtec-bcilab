def setUp(self):
    import os
    ov_binany_path=os.environ['OV_BINARY_PATH']
    self.terminal = App.open("xterm -e " + ov_binany_path +"/ov-externalP300Stimulator.sh ")
    while not self.terminal.window():
        wait(1)
    wait("StartExternalStimulator.png",10)

def test_basic_external_stimulator(self):
    assert(exists("StartExternalStimulator.png"))
    click("StartExternalStimulator.png")
    type("s")
    wait(10)
    type(Key.ESCAPE)


def tearDown(self):
    if self.terminal.window():
        App.close(self.terminal)
        self.terminal= None
