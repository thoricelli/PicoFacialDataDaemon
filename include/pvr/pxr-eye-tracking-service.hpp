#pragma once

#include <binder/Binder.h>
#include <binder/IBinder.h>

using namespace android;

#define SERVICE "pxreyetrackingservice"
#define DESCRIPTOR "pvr.IEyeTrackingService"

#define LISTENER_DESCRIPTOR "pvr.IEyeTrackingServiceListener"

enum TransactionId
{
    INITIALIZE = IBinder::FIRST_CALL_TRANSACTION,
    SET_TRACKING_MODE,
    RESET_TRACKING,
    START,
    STOP,
    START_ALGORITHM,
    SET_ALGORITHM_PARAMETERS,
    GET_ALGORITHM_RESULT,
    STOP_ALGORITHM,
    SET_CAMERA_PARAMETERS,
    ADD_SERVICE_LISTENER,
    REMOVE_SERVICE_LISTENER,
    OPEN_CAMERA,
    START_PREVIEW,
    STOP_PREVIEW,
    CLOSE_CAMERA,
    GET_CAMERA_PARAMETERS,
    GET_TRACKING_DATA_SHARED_MEMORY,
    GET_CAMERA_FRAME_SHARED_MEMORY,
    SET_CAMERA_ERROR_LISTENER,
    ADD_TO_SPECIFIC_LIST,
    SET_DATA,
    GET_DATA,
    GET_PUPIL_DISTANCE,
    HAS_EYE_CAMERA,
    REGISTER_IPD_CALLBACK,
    SET_IPD,
    FINISH_IPD_CALIBRATION
};

enum AlgorithmParameters
{
    EYE_TRACKING_ON = 0x0004,
    FACE_TRACKING_ON = 0x0008
};

class PxrEyeTrackingService
{
public:
    static status_t SetTrackingMode(int mode);
    /**
     * Note: does not seem to work.
     */
    static status_t AddServiceListener(sp<IBinder> binder);
    static status_t GetTrackingDataSharedMemory(int type, void **memory);
    static status_t StartAlgorithm(int camera, int parameters, int timeoutMs);
    static status_t StopAlgorithm(int camera, int parameters);
};