/*
 * \copyright
 * Copyright 1993-2018 NVIDIA Corporation.  All rights reserved.
 * NOTICE TO LICENSEE: This source code and/or documentation ("Licensed Deliverables")
 * are subject to the applicable NVIDIA license agreement
 * that governs the use of the Licensed Deliverables.
*/

#pragma once
#include "NvEncodeAPI/NvEncodeAPI.h"
#include <d3d9.h>
#include <functional>
// Encodering params structure, contains the data needed to initialize
// an encoder.

#define SET_VER(configStruct, type) {configStruct.version = type##_VER;}

#define MAX_BUF_QUEUE 1
#define BITSTREAM_BUFFER_SIZE 2 * 1024 * 1024       // May need larger size for 4K or lossless encode


// Encodering params structure, contains the data needed to initialize
// an encoder.

static char __NVEncodeLibName32[] = "nvEncodeAPI.dll";
static char __NVEncodeLibName64[] = "nvEncodeAPI64.dll";

typedef std::function<void(void* data)> onEncode_fp;
typedef NVENCSTATUS(__stdcall *MYPROC)(NV_ENCODE_API_FUNCTION_LIST *);

typedef struct
{
    unsigned int      dwWidth;
    unsigned int      dwHeight;
    IDirect3DSurface9 *pD3DSurf;
    void              *hInputSurface;
    unsigned int      lockedPitch;
    NV_ENC_BUFFER_FORMAT bufferFmt;
    NV_ENC_INPUT_RESOURCE_TYPE type;
    void              *hRegisteredHandle;
}EncodeInputSurfaceInfo;

typedef struct
{
    unsigned int     dwSize;
    unsigned int     dwBitstreamDataSize;
    void             *hBitstreamBuffer;
    HANDLE           hOutputEvent;
    bool             bWaitOnEvent;
    void             *pBitstreamBufferPtr;
    bool             bEOSFlag;
    bool             bReconfiguredflag;
}EncodeOutputBuffer;

// Encoder class, simple wrapper around the dx9 interface provided by NvEncode SDK
// See https://developer.nvidia.com/sites/default/files/akamai/cuda/files/CUDADownloads/NVENC_VideoEncoder_API_ProgGuide.pdf
// for more details.
// This class demonstrates how to use IDirect3DSurface9* allocated by a client of NvEncodeAPI as input to the encoder
// This class demonstrates using NvEncodeAPI in synchronous mode

class Encoder
{
public:
    HMODULE                                              m_hinstLib;
    HANDLE                                               m_hEncoder;
    NV_ENCODE_API_FUNCTION_LIST                         *m_pEncodeAPI;
    IDirect3DDevice9                                    *m_pD3D9Dev;
    unsigned int                                         m_dwEncodeGUIDCount;
    GUID                                                 m_stEncodeGUID;
    unsigned int                                         m_dwCodecProfileGUIDCount;
    GUID                                                 m_stCodecProfileGUID;
    unsigned int                                         m_dwPresetGUIDCount;
    GUID                                                 m_stPresetGUID;
    unsigned int                                         m_encodersAvailable;
    unsigned int                                         m_dwInputFmtCount;
    NV_ENC_BUFFER_FORMAT                                *m_pAvailableSurfaceFmts;
    NV_ENC_BUFFER_FORMAT                                 m_dwInputFormat;
    NV_ENC_INITIALIZE_PARAMS                             m_stInitEncParams;
    NV_ENC_CONFIG                                        m_stEncodeConfig;
    NV_ENC_PRESET_CONFIG                                 m_stPresetConfig;
    NV_ENC_PIC_PARAMS                                    m_stEncodePicParams;
    NV_ENC_RECONFIGURE_PARAMS                            m_stReconfigureParam;
    EncodeInputSurfaceInfo                               m_stInputSurface[MAX_BUF_QUEUE];
    EncodeOutputBuffer                                   m_stBitstreamBuffer[MAX_BUF_QUEUE];
    unsigned int                                         m_dwWidth;
    unsigned int                                         m_dwHeight;
    bool                                                 m_bUseMappedResource;
    unsigned int                                         m_dwMaxBufferSize;
    unsigned int                                         m_dwFrameCount;
    unsigned int                                         m_dwMaxWidth;
    unsigned int                                         m_dwMaxHeight;
	bool                                                 m_bIsYUV444;
	bool                                                 m_bIsHEVC;
    bool                                                 m_bIs10bitRGBCapture;
    bool                                                 m_bEnableEmphasisLevelMap;
    BYTE *                                               m_pEmphasisMap;

public:
    Encoder();
    ~Encoder();
public:
    HRESULT Init(IDirect3DDevice9 *pD3D9Dev);
    HRESULT SetupEncoder(unsigned int dwWidth, unsigned int dwHeight, unsigned int dwBitRate, unsigned int maxWidth, unsigned int maxHeight, 
		                 unsigned int codec, IDirect3DSurface9 **ppInputSurfs, bool bYUV444, bool b10bitRGBCapture, bool bEnableEmphasisMap, BYTE* pEmphasisMap, bool bConstQP, unsigned int QP);
    HRESULT LaunchEncode(unsigned int i);
    HRESULT GetBitstream(unsigned int i);
    HRESULT TearDown();
    HRESULT Reconfigure(unsigned int dwWidth, unsigned int dwHeight, unsigned int dwBitRate);
	void SetOnEncoded(onEncode_fp fp);
private:
    HRESULT AllocateOPBufs();
    HRESULT Flush();
    HRESULT ReleaseIPBufs();
    HRESULT ReleaseOPBufs();
    HRESULT LoadEncAPI();
    HRESULT ValidateParams();
    inline BOOL Is64Bit() { return (sizeof(void *) != sizeof(DWORD)); }

	onEncode_fp onEncoded;
};