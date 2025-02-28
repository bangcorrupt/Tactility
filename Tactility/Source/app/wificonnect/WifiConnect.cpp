#include "WifiConnect.h"

#include "app/AppContext.h"
#include "TactilityCore.h"
#include "service/loader/Loader.h"
#include "service/wifi/Wifi.h"
#include "lvgl/LvglSync.h"

namespace tt::app::wificonnect {

#define TAG "wifi_connect"

extern const AppManifest manifest;

/** Returns the app data if the app is active. Note that this could clash if the same app is started twice and a background thread is slow. */
std::shared_ptr<WifiConnect> _Nullable optWifiConnect() {
    app::AppContext* app = service::loader::getCurrentApp();
    if (app->getManifest().id == manifest.id) {
        return std::static_pointer_cast<WifiConnect>(app->getData());
    } else {
        return nullptr;
    }
}

static void eventCallback(const void* message, void* context) {
    auto* event = static_cast<const service::wifi::WifiEvent*>(message);
    auto* wifi = static_cast<WifiConnect*>(context);
    State& state = wifi->getState();
    switch (event->type) {
        case service::wifi::WifiEventTypeConnectionFailed:
            if (state.isConnecting()) {
                state.setConnecting(false);
                state.setConnectionError(true);
                wifi->requestViewUpdate();
            }
            break;
        case service::wifi::WifiEventTypeConnectionSuccess:
            if (wifi->getState().isConnecting()) {
                state.setConnecting(false);
                service::loader::stopApp();
            }
            break;
        default:
            break;
    }
    wifi->requestViewUpdate();
}

static void onConnect(const service::wifi::settings::WifiApSettings* ap_settings, bool remember, TT_UNUSED void* parameter) {
    auto* wifi = static_cast<WifiConnect*>(parameter);
    wifi->getState().setApSettings(ap_settings);
    wifi->getState().setConnecting(true);
    service::wifi::connect(ap_settings, remember);
}

WifiConnect::WifiConnect() {
    auto wifi_pubsub = service::wifi::getPubsub();
    wifiSubscription = tt_pubsub_subscribe(wifi_pubsub, &eventCallback, this);
    bindings = (Bindings) {
        .onConnectSsid = onConnect,
        .onConnectSsidContext = this,
    };
}

WifiConnect::~WifiConnect() {
    auto pubsub = service::wifi::getPubsub();
    tt_pubsub_unsubscribe(pubsub, wifiSubscription);
}

void WifiConnect::lock() {
    tt_check(mutex.acquire(TtWaitForever) == TtStatusOk);
}

void WifiConnect::unlock() {
    tt_check(mutex.release() == TtStatusOk);
}

void WifiConnect::requestViewUpdate() {
    lock();
    if (view_enabled) {
        if (lvgl::lock(1000)) {
            view.update();
            lvgl::unlock();
        } else {
            TT_LOG_E(TAG, LOG_MESSAGE_MUTEX_LOCK_FAILED_FMT, "LVGL");
        }
    }
    unlock();
}

void WifiConnect::onShow(AppContext& app, lv_obj_t* parent) {
    lock();
    view_enabled = true;
    view.init(app, parent);
    view.update();
    unlock();
}

void WifiConnect::onHide(TT_UNUSED AppContext& app) {
    // No need to lock view, as this is called from within Gui's LVGL context
    lock();
    view_enabled = false;
    unlock();
}

static void onShow(AppContext& app, lv_obj_t* parent) {
    auto wifi = std::static_pointer_cast<WifiConnect>(app.getData());
    wifi->onShow(app, parent);
}

static void onHide(AppContext& app) {
    auto wifi = std::static_pointer_cast<WifiConnect>(app.getData());
    wifi->onHide(app);
}

static void onStart(AppContext& app) {
    auto wifi = std::make_shared<WifiConnect>();
    app.setData(wifi);
}


extern const AppManifest manifest = {
    .id = "WifiConnect",
    .name = "Wi-Fi Connect",
    .icon = LV_SYMBOL_WIFI,
    .type = TypeSettings,
    .onStart = &onStart,
    .onShow = &onShow,
    .onHide = &onHide
};

} // namespace
