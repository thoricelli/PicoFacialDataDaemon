#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/Status.h>
#include <binder/IServiceManager.h>
#include <sys/mman.h>

#include "pvr/pxr-eye-tracking-service.hpp"

status_t PxrEyeTrackingService::SetTrackingMode(int mode)
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> eyeTrackingBinder = sm->getService(String16(SERVICE));

    Parcel message;
    Parcel reply;

    message.writeInterfaceToken(String16(DESCRIPTOR));
    message.writeInt32(mode);

    status_t binderStatus = eyeTrackingBinder->transact(
        SET_TRACKING_MODE,
        message,
        &reply,
        0);

    if (binderStatus != OK)
        return binderStatus;

    return reply.readInt32();
}

status_t PxrEyeTrackingService::GetTrackingDataSharedMemory(int type, int *fd, void **memory)
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> eyeTrackingBinder = sm->getService(String16(SERVICE));

    Parcel message;
    Parcel reply;

    message.writeInterfaceToken(String16(DESCRIPTOR));
    message.writeInt32(type);

    status_t transactStatus = eyeTrackingBinder->transact(
        GET_TRACKING_DATA_SHARED_MEMORY,
        message,
        &reply,
        0);

    if (transactStatus != OK)
        return transactStatus;

    status_t binderStatus = reply.readInt32();
    if (binderStatus != OK)
        return binderStatus;

    status_t serviceStatus = reply.readInt32();
    if (serviceStatus != OK)
        return serviceStatus;

    int hasPacket = reply.readInt32();
    if (!hasPacket)
        return FAILED_TRANSACTION;

    base::unique_fd uniqueFd;
    status_t hasFd = reply.readUniqueFileDescriptor(&uniqueFd);

    if (hasFd != OK)
        return hasFd;

    int memorySize = reply.readInt32();

    if (memorySize <= 0)
        return FAILED_TRANSACTION;

    int fdSharedMemory = uniqueFd.get();
    *fd = fdSharedMemory;

    void *sharedMemory = mmap(
        nullptr,
        memorySize,
        PROT_READ,
        MAP_SHARED,
        fdSharedMemory,
        0);

    if (sharedMemory == MAP_FAILED)
        return FAILED_TRANSACTION;

    *memory = sharedMemory;

    return OK;
}

status_t PxrEyeTrackingService::AddServiceListener(sp<IBinder> binder)
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> eyeTrackingBinder = sm->getService(String16(SERVICE));

    Parcel message;
    Parcel reply;

    message.writeInterfaceToken(String16(DESCRIPTOR));
    status_t sbStatus = message.writeStrongBinder(binder);

    status_t binderStatus = eyeTrackingBinder->transact(
        ADD_SERVICE_LISTENER,
        message,
        &reply,
        0);

    if (binderStatus != OK)
        return binderStatus;

    return reply.readInt32();
}

status_t PxrEyeTrackingService::StartAlgorithm(int camera, int parameters, int timeoutMs)
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> eyeTrackingBinder = sm->getService(String16(SERVICE));

    Parcel message;
    Parcel reply;

    message.writeInterfaceToken(String16(DESCRIPTOR));

    message.writeInt32(camera);
    message.writeString16(String16(std::to_string(parameters).c_str()));
    message.writeInt32(timeoutMs);

    status_t transactStatus = eyeTrackingBinder->transact(
        START_ALGORITHM,
        message,
        &reply,
        0);

    if (transactStatus != OK)
        return transactStatus;

    status_t binderStatus = reply.readInt32();

    if (binderStatus != OK)
        return binderStatus;

    status_t functionStatus = reply.readInt32();

    if (functionStatus != OK)
        return functionStatus;

    return OK;
}

status_t PxrEyeTrackingService::StopAlgorithm(int camera, int parameters)
{
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> eyeTrackingBinder = sm->getService(String16(SERVICE));

    Parcel message;
    Parcel reply;

    message.writeInterfaceToken(String16(DESCRIPTOR));

    message.writeInt32(camera);
    message.writeInt32(parameters);

    status_t transactStatus = eyeTrackingBinder->transact(
        STOP_ALGORITHM,
        message,
        &reply,
        0);

    if (transactStatus != OK)
        return transactStatus;

    status_t binderStatus = reply.readInt32();

    if (binderStatus != OK)
        return binderStatus;

    status_t functionStatus = reply.readInt32();

    if (functionStatus != OK)
        return functionStatus;

    return OK;
}