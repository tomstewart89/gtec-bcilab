 -- ___________________________________________________________________ --
 --                                                                     --
 --  OpenViBE toolkit stimulation identifiers                           --
 -- ___________________________________________________________________ --
 --                                                                     --
 --  Stimulation codes                                                  --
 --  Originally from openvibe-toolkit/ovtk_defines.h                    --

OVTK_StimulationId_ExperimentStart                    = 0x00008001
OVTK_StimulationId_ExperimentStop                     = 0x00008002
OVTK_StimulationId_SegmentStart                       = 0x00008003
OVTK_StimulationId_SegmentStop                        = 0x00008004
OVTK_StimulationId_TrialStart                         = 0x00008005
OVTK_StimulationId_TrialStop                          = 0x00008006
OVTK_StimulationId_BaselineStart                      = 0x00008007
OVTK_StimulationId_BaselineStop                       = 0x00008008
OVTK_StimulationId_RestStart                          = 0x00008009
OVTK_StimulationId_RestStop                           = 0x0000800a
OVTK_StimulationId_VisualStimulationStart             = 0x0000800b
OVTK_StimulationId_VisualStimulationStop              = 0x0000800c
OVTK_StimulationId_VisualSteadyStateStimulationStart  = 0x00008010
OVTK_StimulationId_VisualSteadyStateStimulationStop   = 0x00008011

OVTK_StimulationId_LabelStart                         = 0x00008100
OVTK_StimulationId_Label_00                           = 0x00008100
OVTK_StimulationId_Label_01                           = 0x00008101
OVTK_StimulationId_Label_02                           = 0x00008102
OVTK_StimulationId_Label_03                           = 0x00008103
OVTK_StimulationId_Label_04                           = 0x00008104
OVTK_StimulationId_Label_05                           = 0x00008105
OVTK_StimulationId_Label_06                           = 0x00008106
OVTK_StimulationId_Label_07                           = 0x00008107
OVTK_StimulationId_Label_08                           = 0x00008108
OVTK_StimulationId_Label_09                           = 0x00008109
OVTK_StimulationId_Label_0A                           = 0x0000810a
OVTK_StimulationId_Label_0B                           = 0x0000810b
OVTK_StimulationId_Label_0C                           = 0x0000810c
OVTK_StimulationId_Label_0D                           = 0x0000810d
OVTK_StimulationId_Label_0E                           = 0x0000810e
OVTK_StimulationId_Label_0F                           = 0x0000810f
OVTK_StimulationId_Label_10                           = 0x00008110
OVTK_StimulationId_Label_11                           = 0x00008111
OVTK_StimulationId_Label_12                           = 0x00008112
OVTK_StimulationId_Label_13                           = 0x00008113
OVTK_StimulationId_Label_14                           = 0x00008114
OVTK_StimulationId_Label_15                           = 0x00008115
OVTK_StimulationId_Label_16                           = 0x00008116
OVTK_StimulationId_Label_17                           = 0x00008117
OVTK_StimulationId_Label_18                           = 0x00008118
OVTK_StimulationId_Label_19                           = 0x00008119
OVTK_StimulationId_Label_1A                           = 0x0000811a
OVTK_StimulationId_Label_1B                           = 0x0000811b
OVTK_StimulationId_Label_1C                           = 0x0000811c
OVTK_StimulationId_Label_1D                           = 0x0000811d
OVTK_StimulationId_Label_1E                           = 0x0000811e
OVTK_StimulationId_Label_1F                           = 0x0000811f
OVTK_StimulationId_LabelEnd                           = 0x000081ff

OVTK_StimulationId_Train                              = 0x00008201
OVTK_StimulationId_Beep                               = 0x00008202
OVTK_StimulationId_DoubleBeep                         = 0x00008203
OVTK_StimulationId_EndOfFile                          = 0x00008204
OVTK_StimulationId_Target                             = 0x00008205
OVTK_StimulationId_NonTarget                          = 0x00008206
OVTK_StimulationId_TrainCompleted                     = 0x00008207
OVTK_StimulationId_Reset                              = 0x00008208

 -- ___________________________________________________________________ --
 --                                                                     --
 --  GDF file format stimulation identifiers                            --
 -- ___________________________________________________________________ --
 --                                                                     --

OVTK_GDF_Artifact_EOG_Large                                = 0x101
OVTK_GDF_Artifact_ECG                                      = 0x102
OVTK_GDF_Artifact_EMG                                      = 0x103
OVTK_GDF_Artifact_Movement                                 = 0x104
OVTK_GDF_Artifact_Failing_Electrode                        = 0x105
OVTK_GDF_Artifact_Sweat                                    = 0x106
OVTK_GDF_Artifact_50_60_Hz_Interference                    = 0x107
OVTK_GDF_Artifact_Breathing                                = 0x108
OVTK_GDF_Artifact_Pulse                                    = 0x109
OVTK_GDF_Artifact_EOG_Small                                = 0x10A

OVTK_GDF_Calibration                                       = 0x10F

OVTK_GDF_EEG_Sleep_Splindles                               = 0x111
OVTK_GDF_EEG_K_Complexes                                   = 0x112
OVTK_GDF_EEG_Saw_Tooth_Waves                               = 0x113
OVTK_GDF_EEG_Idling_EEG_Eyes_Open                          = 0x114
OVTK_GDF_EEG_Idling_EEG_Eyes_Closed                        = 0x115
OVTK_GDF_EEG_Spike                                         = 0x116
OVTK_GDF_EEG_Seizure                                       = 0x117

OVTK_GDF_VEP                                               = 0x121
OVTK_GDF_AEP                                               = 0x122
OVTK_GDF_SEP                                               = 0x123
OVTK_GDF_TMS                                               = 0x12F

OVTK_GDF_SSVEP                                             = 0x131
OVTK_GDF_SSAEP                                             = 0x132
OVTK_GDF_SSSEP                                             = 0x133

OVTK_GDF_Start_Of_Trial                                    = 0x300
OVTK_GDF_Left                                              = 0x301
OVTK_GDF_Right                                             = 0x302
OVTK_GDF_Foot                                              = 0x303
OVTK_GDF_Tongue                                            = 0x304
OVTK_GDF_class5                                            = 0x305
OVTK_GDF_Down                                              = 0x306
OVTK_GDF_class7                                            = 0x307
OVTK_GDF_class8                                            = 0x308
OVTK_GDF_class9                                            = 0x309
OVTK_GDF_class10                                           = 0x30A
OVTK_GDF_class11                                           = 0x30B
OVTK_GDF_Up                                                = 0x30C
OVTK_GDF_Feedback_Continuous                               = 0x30D
OVTK_GDF_Feedback_Discrete                                 = 0x30E
OVTK_GDF_Cue_Unknown_Undefined                             = 0x30F
OVTK_GDF_Beep                                              = 0x311
OVTK_GDF_Cross_On_Screen                                   = 0x312
OVTK_GDF_Flashing_Light                                    = 0x313
 --  SPECIALY ADDED BY YR
OVTK_GDF_End_Of_Trial                                      = 0x320

OVTK_GDF_Correct                                           = 0x381
OVTK_GDF_Incorrect                                         = 0x382
 --  SPECIALY ADDED BY YR
OVTK_GDF_End_Of_Session                                    = 0x3F2
OVTK_GDF_Rejection                                         = 0x3FF

OVTK_GDF_OAHE                                              = 0x401
OVTK_GDF_RERA                                              = 0x402
OVTK_GDF_CAHE                                              = 0x403
OVTK_GDF_CSB                                               = 0x404
OVTK_GDF_Sleep_Hypoventilation                             = 0x405
OVTK_GDF_Maximum_Inspiration                               = 0x40E
OVTK_GDF_Start_Of_Inspiration                              = 0x40F

OVTK_GDF_Wake                                              = 0x410
OVTK_GDF_Stage_1                                           = 0x411
OVTK_GDF_Stage_2                                           = 0x412
OVTK_GDF_Stage_3                                           = 0x413
OVTK_GDF_Stage_4                                           = 0x414
OVTK_GDF_REM                                               = 0x415

OVTK_GDF_Lights_On                                         = 0x420
OVTK_GDF_Lights_Off                                        = 0x8420

OVTK_GDF_Eyes_Left                                         = 0x431
OVTK_GDF_Eyes_Right                                        = 0x432
OVTK_GDF_Eyes_Up                                           = 0x433
OVTK_GDF_Eyes_Down                                         = 0x434
OVTK_GDF_Horizontal_Eye_Movement                           = 0x435
OVTK_GDF_Vertical_Eye_Movement                             = 0x436
OVTK_GDF_Rotation_Clockwise                                = 0x437
OVTK_GDF_Rotation_Counterclockwise                         = 0x438
OVTK_GDF_Eye_Blink                                         = 0x439

OVTK_GDF_Left_Hand_Movement                                = 0x441
OVTK_GDF_Right_Hand_Movement                               = 0x442
OVTK_GDF_Head_Movement                                     = 0x443
OVTK_GDF_Tongue_Movement                                   = 0x444
OVTK_GDF_Swallowing                                        = 0x445
OVTK_GDF_Biting                                            = 0x446
OVTK_GDF_Foot_Movement                                     = 0x447
OVTK_GDF_Foot_Right_Movement                               = 0x448
OVTK_GDF_Arm_Movement                                      = 0x449
OVTK_GDF_Arm_Right_Movement                                = 0x44A

OVTK_GDF_ECG_Fiducial_Point_QRS_Complex                    = 0x501
OVTK_GDF_ECG_P_Wave                                        = 0x502
OVTK_GDF_ECG_QRS_Complex                                   = 0x503
OVTK_GDF_ECG_R_Point                                       = 0x504
OVTK_GDF_ECG_T_Wave                                        = 0x506
OVTK_GDF_ECG_U_Wave                                        = 0x507

OVTK_GDF_Start                                             = 0x580
OVTK_GDF_25_Watt                                           = 0x581
OVTK_GDF_50_Watt                                           = 0x582
OVTK_GDF_75_Watt                                           = 0x583
OVTK_GDF_100_Watt                                          = 0x584
OVTK_GDF_125_Watt                                          = 0x585
OVTK_GDF_150_Watt                                          = 0x586
OVTK_GDF_175_Watt                                          = 0x587
OVTK_GDF_200_Watt                                          = 0x588
OVTK_GDF_225_Watt                                          = 0x589
OVTK_GDF_250_Watt                                          = 0x58A
OVTK_GDF_275_Watt                                          = 0x58B
OVTK_GDF_300_Watt                                          = 0x58C
OVTK_GDF_325_Watt                                          = 0x58D
OVTK_GDF_350_Watt                                          = 0x58E

OVTK_GDF_Start_Of_New_Segment                              = 0x7FFE
OVTK_GDF_Non_Equidistant_Sampling_Value                    = 0x7FFF
