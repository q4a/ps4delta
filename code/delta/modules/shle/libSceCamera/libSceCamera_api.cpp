
// Copyright (C) 2019 Force67

// This file was generated on Sat Sep  7 22:01:24 2019

#include "../../ModuleLinker.h"

#include "libSceCamera.h"

static const mlink::FunctionInfo functions[] = {
	{0x7E4644EC7BA9DABA, &sceCameraAudioGetData},
	{0x85FB42E40D42F0E4, &sceCameraAudioGetData2},
	{0x0E1AAA162054FBA8, &sceCameraAudioOpen},
	{0x634A420DA8F39154, &sceCameraChangeAppModuleState},
	{0x38C4BD2E572BBC1A, &sceCameraClose},
	{0xCEDA87E6A353A539, &sceCameraCloseByHandle},
	{0x9C11FA8B6B381A57, &sceCameraDeviceOpen},
	{0xD1BB483C3E618390, &sceCameraGetAttribute},
	{0xA048BABCCFF71367, &sceCameraGetAutoExposureGain},
	{0xA933D1321E1E63AD, &sceCameraGetAutoWhiteBalance},
	{0x8470357EB94CC581, &sceCameraGetCalibData},
	{0x447609EC628E48C8, &sceCameraGetCalibrationData},
	{0x65AAA6184B58B8BD, &sceCameraGetConfig},
	{0x6B9C45B9E31920CB, &sceCameraGetContrast},
	{0xB6C942BA4A8513E1, &sceCameraGetDefectivePixelCancellation},
	{0x0D238B0AB7372A1F, &sceCameraGetDeviceConfig},
	{0x599A719D200CFDDB, &sceCameraGetDeviceInfo},
	{0x39B22CB5EEE1A9D9, &sceCameraGetExposureGain},
	{0x9B180C991FB52ABD, &sceCameraGetFrameData},
	{0x595A31DABC06B927, &sceCameraGetGamma},
	{0xCEB2140CA671D221, &sceCameraGetHue},
	{0x5EA6111DCE1AC37C, &sceCameraGetLensCorrection},
	{0x4530CEB165AA74C1, &sceCameraGetSaturation},
	{0x73A169F4CD445D77, &sceCameraGetSharpness},
	{0x1D7E79DB8139B4C6, &sceCameraGetWhiteBalance},
	{0xA7A9F73698B7618E, &sceCameraIsAttached},
	{0x537055C109764794, &sceCameraIsValidFrameData},
	{0x0479FCDF1AC5F761, &sceCameraOpen},
	{0x793CB03925AC1222, &sceCameraOpenByModuleId},
	{0xA72F29EA465C1E60, &sceCameraRemoveAppModuleFocus},
	{0x8F98AC15522564B9, &sceCameraSetAppModuleFocus},
	{0x7683E57F7DDA6FF5, &sceCameraSetAttribute},
	{0xF7A17BCE9D57A3E9, &sceCameraSetAttributeInternal},
	{0xC9F49DB300DA125A, &sceCameraSetAutoExposureGain},
	{0xCC828BE246657AE7, &sceCameraSetAutoWhiteBalance},
	{0x2C4324E5C4C72840, &sceCameraSetCalibData},
	{0x550FB9900AAC1364, &sceCameraSetConfig},
	{0xF7E48D85B72D93AE, &sceCameraSetConfigInternal},
	{0xDE2E4C133AC2D698, &sceCameraSetContrast},
	{0xBDE8E8B84BAC0BB8, &sceCameraSetDebugStop},
	{0x8CCBF8D32D80DB78, &sceCameraSetDefectivePixelCancellation},
	{0xBC44777083011EA2, &sceCameraSetDefectivePixelCancellationInternal},
	{0xC2004C5C9240E8AE, &sceCameraSetExposureGain},
	{0x8DE4E953432A294D, &sceCameraSetForceActivate},
	{0x961108B07CC1F2BE, &sceCameraSetGamma},
	{0x408F06549532D996, &sceCameraSetHue},
	{0x2BB5BB1F8651C1B7, &sceCameraSetLensCorrection},
	{0x7876B7BE11AEDAB4, &sceCameraSetLensCorrectionInternal},
	{0x355213B8AF3767BA, &sceCameraSetProcessFocusByHandle},
	{0x6D22848B63F3CD72, &sceCameraSetSaturation},
	{0x3FFECC55FCEFA6C3, &sceCameraSetSharpness},
	{0xDD524EA732A821E3, &sceCameraSetTrackerMode},
	{0xC297B217027E5042, &sceCameraSetVideoSync},
	{0xF16B66AA613879DC, &sceCameraSetVideoSyncInternal},
	{0x937CCF21C80536FD, &sceCameraSetWhiteBalance},
	{0xF44A5160CCBBAC75, &sceCameraStart},
	{0x70BC45D50B47721D, &sceCameraStartByHandle},
	{0xD86D82D2799DFBE3, &sceCameraStop},
	{0xF97D4A8279F76F38, &sceCameraStopByHandle},
	{0x076EB4A3DA52CCCF, &unk_B260o9pSzM8},
	{0x200CF61E06505B31, &unk_IAz2HgZQWzE},
	{0x4218EB3E444F5194, &unk_QhjrPkRPUZQ},
	{0x505A272FBC68A453, &unk_UFonL7xopFM},
	{0x50BC5BC2A89862E5, &unk_ULxbwqiYYuU},
	{0x85AC0A6A4F80BB0E, &unk_hawKak_Auw4},
	{0x8D3242772BFD18B5, &unk_jTJCdyv9GLU},
	{0x952D2D33A9FE4391, &unk_lS0tM6n_Q5E},
	{0x9E747B2802033EFF, &unk_nnR7KAIDPv8},
	{0x9FEAC578FD575F23, &unk_n_rFeP1XXyM},
	{0xA25A23619298898B, &unk_olojYZKYiYs},
	{0xC107DDEE47D166FA, &unk_wQfd7kfRZvo},
	{0xC3253DF045C06315, &unk_wyU98EXAYxU},
	{0xD309DFD9AEB416A2, &unk_0wnf2a60FqI},
	{0xE4E89EE5102B7D6B, &unk_5Oie5RArfWs},
	{0xF0C8CED39AA4E610, &unk_8MjO05qk5hA},
	{0xFC7DD4C0642F3592, &unk__H3UwGQvNZI},
};

MODULE_INIT(libSceCamera);