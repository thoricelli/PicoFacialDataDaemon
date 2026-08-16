#include "log.hpp"

#include <binder/Binder.h>
#include <binder/IBinder.h>
#include <utils/RefBase.h>
#include <binder/Parcel.h>
#include <thread>

#include "facial-tracking.hpp"

// TODO seperate into its OWN file!
enum EyeTrackingServiceListenerTransactions
{
    ON_FRAME_AVAILABLE = IBinder::FIRST_CALL_TRANSACTION,
    ON_ALGORITHM_RESULTS_AVAILABLE,
    ON_DEVICE_ERROR,
    ON_IPD_AVAILABLE,
    ON_GLASS_WEARABLE_AVAILABLE,
    ON_IPD_FULL_DATA_AVAILABLE
};

class EyeTrackingServiceListener : public BBinder
{
public:
    EyeTrackingServiceListener(FacialTracking *facialTracking)
    {
        this->facialTracking = facialTracking;
    }
    status_t onTransact(unsigned int code, const Parcel &data, Parcel *reply, unsigned int flags = 0) override
    {
        printf("onTransact, code: %d", code);

        data.enforceInterface(String16(LISTENER_DESCRIPTOR));

        switch (code)
        {
        case ON_ALGORITHM_RESULTS_AVAILABLE:
            int unknownData = data.readInt32();

            printf("We received algorithm result, with param_1 = %d", unknownData);

            // this->facialTracking->OnAlgorithmResultAvailable();

            reply->writeInt32(OK);

            return NO_ERROR;
        }

        return BBinder::onTransact(code, data, reply, flags);
    }

private:
    FacialTracking *facialTracking;
};

FacialTracking::FacialTracking()
{
    this->eyeTrackingServiceListener = new EyeTrackingServiceListener(this);
}

bool FacialTracking::Start()
{
    status_t algorithmStatus = PxrEyeTrackingService::StartAlgorithm(5, EYE_TRACKING_ON | FACE_TRACKING_ON, 1000);

    if (algorithmStatus != OK)
        return false;

    // If someone can figure out why the service listener isn't working, please send a PR... for now I have to resort to polling the shared memory...
    // status_t status = PxrEyeTrackingService::AddServiceListener(this->eyeTrackingServiceListener);

    if (this->faceTrackingDataBuffer)
        this->faceTrackingDataBuffer->Close();

    if (this->eyeTrackingDataBuffer)
        this->eyeTrackingDataBuffer->Close();

    //  Face tracking data buffer.
    void *faceTrackingSharedMemory = nullptr;
    int faceTrackingDataBufferFd;
    status_t sharedMemoryStatus = PxrEyeTrackingService::GetTrackingDataSharedMemory(SHARED_MEMORY_FACE_TRACKING, &faceTrackingDataBufferFd, &faceTrackingSharedMemory);

    if (sharedMemoryStatus != OK)
        return false;

    void *eyeTrackingSharedMemory = nullptr;
    int eyeTrackingDataBufferFd;
    sharedMemoryStatus = PxrEyeTrackingService::GetTrackingDataSharedMemory(SHARED_MEMORY_EYE_TRACKING, &eyeTrackingDataBufferFd, &eyeTrackingSharedMemory);

    if (sharedMemoryStatus != OK)
        return false;

    this->faceTrackingDataBuffer = new DataBuffer(faceTrackingSharedMemory, faceTrackingDataBufferFd);
    this->eyeTrackingDataBuffer = new DataBuffer(eyeTrackingSharedMemory, eyeTrackingDataBufferFd);

    return true;
}

bool FacialTracking::Stop()
{
    return PxrEyeTrackingService::StopAlgorithm(5, EYE_TRACKING_ON | FACE_TRACKING_ON) == OK;
}

void FacialTracking::GetFacialData(PxrFTInfo **faceTrackingData, pxr_eyepose_data_v2_0 **eyeTrackingData)
{
    *faceTrackingData = static_cast<PxrFTInfo *>(this->faceTrackingDataBuffer->GetLatest());
    *eyeTrackingData = static_cast<pxr_eyepose_data_v2_0 *>(this->eyeTrackingDataBuffer->GetLatest());
}