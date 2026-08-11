#pragma once

#include <binder/Binder.h>

#include <cstdint>
#include <chrono>

#include "pvr/pxr-eye-tracking-service.hpp"
#include "data-buffer.hpp"

#define BLEND_SHAPE_NUMS 72
typedef struct
{
    int64_t timestamp;
    float blendShapeWeight[BLEND_SHAPE_NUMS];
    float videoInputValid[10];
    float laughingProb;
    float emotionProb[10];
} PxrFTInfo;

typedef struct
{
    int64_t timestamp;

    int32_t leftEyePoseStatus;     //!< Bit field (pvrEyePoseStatus) indicating left eye pose status
    int32_t rightEyePoseStatus;    //!< Bit field (pvrEyePoseStatus) indicating right eye pose status
    int32_t combinedEyePoseStatus; //!< Bit field (pvrEyePoseStatus) indicating combined eye pose status

    float leftEyeGazePoint[3];     //!< Left Eye Gaze Point
    float rightEyeGazePoint[3];    //!< Right Eye Gaze Point
    float combinedEyeGazePoint[3]; //!< Combined Eye Gaze Point (HMD center-eye point)

    float leftEyeGazeVector[3];     //!< Left Eye Gaze Point
    float rightEyeGazeVector[3];    //!< Right Eye Gaze Point
    float combinedEyeGazeVector[3]; //!< Combined Eye Gaze Vector (HMD center-eye point)

    float leftEyeOpenness;  //!< Left eye value between 0.0 and 1.0 where 1.0 means fully open and 0.0 closed.
    float rightEyeOpenness; //!< Right eye value between 0.0 and 1.0 where 1.0 means fully open and 0.0 closed.

    float leftEyePupilDilation;  //!< Left eye value in millimeters indicating the pupil dilation
    float rightEyePupilDilation; //!< Right eye value in millimeters indicating the pupil dilation

    float leftEyePositionGuide[3];     //!< Position of the inner corner of the left eye in meters from the HMD center-eye coordinate system's origin.
    float rightEyePositionGuide[3];    //!< Position of the inner corner of the right eye in meters from the HMD center-eye coordinate system's origin.
    float foveatedGazeDirection[3];    //!< Position of the gaze direction in meters from the HMD center-eye coordinate system's origin.
    int32_t foveatedGazeTrackingState; //!< The current state of the foveatedGazeDirection signal.

} pxr_eyepose_data_v2_0;

enum DataSharedMemorySlot
{
    SHARED_MEMORY_EYE_TRACKING_OLD = 1,
    SHARED_MEMORY_EYE_TRACKING,
    SHARED_MEMORY_FACE_TRACKING
};

class FacialTracking
{
public:
    FacialTracking();
    bool Start();
    bool Stop();
    void GetFacialData(PxrFTInfo **faceTrackingData, pxr_eyepose_data_v2_0 **eyeTrackingData);

private:
    sp<IBinder> eyeTrackingServiceListener;

    DataBuffer *faceTrackingDataBuffer;
    DataBuffer *eyeTrackingDataBuffer;
};