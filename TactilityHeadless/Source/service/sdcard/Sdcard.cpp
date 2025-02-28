#include "Mutex.h"
#include "Timer.h"

#include "service/ServiceContext.h"
#include "TactilityCore.h"
#include "TactilityHeadless.h"
#include "service/ServiceRegistry.h"

#include <cstdlib>

#define TAG "sdcard_service"

namespace tt::service::sdcard {

extern const ServiceManifest manifest;

struct ServiceData {
    Mutex mutex;
    std::unique_ptr<Timer> updateTimer;
    hal::SdCard::State lastState = hal::SdCard::StateUnmounted;

    bool lock(TickType_t timeout) const {
        return mutex.acquire(timeout) == TtStatusOk;
    }

    void unlock() const {
        tt_check(mutex.release() == TtStatusOk);
    }
};


static void onUpdate(std::shared_ptr<void> context) {
    auto sdcard = tt::hal::getConfiguration()->sdcard;
    if (sdcard == nullptr) {
        return;
    }

    auto data = std::static_pointer_cast<ServiceData>(context);

    if (!data->lock(50)) {
        TT_LOG_W(TAG, LOG_MESSAGE_MUTEX_LOCK_FAILED);
        return;
    }

    auto new_state = sdcard->getState();

    if (new_state == hal::SdCard::StateError) {
        TT_LOG_W(TAG, "Sdcard error - unmounting. Did you eject the card in an unsafe manner?");
        sdcard->unmount();
    }

    if (new_state != data->lastState) {
        data->lastState = new_state;
    }

    data->unlock();
}

static void onStart(ServiceContext& service) {
    if (hal::getConfiguration()->sdcard != nullptr) {
        auto data = std::make_shared<ServiceData>();
        service.setData(data);

        data->updateTimer = std::make_unique<Timer>(Timer::TypePeriodic, onUpdate, data);
        // We want to try and scan more often in case of startup or scan lock failure
        data->updateTimer->start(1000);
    } else {
        TT_LOG_I(TAG, "task not started due to config");
    }
}

static void onStop(ServiceContext& service) {
    auto data = std::static_pointer_cast<ServiceData>(service.getData());
    if (data->updateTimer != nullptr) {
        // Stop thread
        data->updateTimer->stop();
        data->updateTimer = nullptr;
    }
}

extern const ServiceManifest manifest = {
    .id = "sdcard",
    .onStart = onStart,
    .onStop = onStop
};

} // namespace
