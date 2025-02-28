#include <StringUtils.h>
#include "lvgl.h"
#include "lvgl/Toolbar.h"
#include "service/loader/Loader.h"
#include "app/timezone/TimeZone.h"
#include "Assets.h"
#include "Tactility.h"
#include "time/Time.h"
#include "lvgl/LvglSync.h"

#define TAG "text_viewer"

namespace tt::app::timedatesettings {

extern const AppManifest manifest;

struct Data {
    Mutex mutex = Mutex(Mutex::TypeRecursive);
    lv_obj_t* regionLabelWidget = nullptr;
};

/** Returns the app data if the app is active. Note that this could clash if the same app is started twice and a background thread is slow. */
std::shared_ptr<Data> _Nullable optData() {
    app::AppContext* app = service::loader::getCurrentApp();
    if (app->getManifest().id == manifest.id) {
        return std::static_pointer_cast<Data>(app->getData());
    } else {
        return nullptr;
    }
}

static void onConfigureTimeZonePressed(TT_UNUSED lv_event_t* event) {
    timezone::start();
}

static void onTimeFormatChanged(lv_event_t* event) {
    auto* widget = lv_event_get_target_obj(event);
    bool show_24 = lv_obj_has_state(widget, LV_STATE_CHECKED);
    time::setTimeFormat24Hour(show_24);
}

static void onShow(AppContext& app, lv_obj_t* parent) {
    auto data = std::static_pointer_cast<Data>(app.getData());

    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    lvgl::toolbar_create(parent, app);

    auto* main_wrapper = lv_obj_create(parent);
    lv_obj_set_flex_flow(main_wrapper, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_width(main_wrapper, LV_PCT(100));
    lv_obj_set_flex_grow(main_wrapper, 1);

    auto* region_wrapper = lv_obj_create(main_wrapper);
    lv_obj_set_width(region_wrapper, LV_PCT(100));
    lv_obj_set_height(region_wrapper, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(region_wrapper, 0, 0);
    lv_obj_set_style_border_width(region_wrapper, 0, 0);

    auto* region_prefix_label = lv_label_create(region_wrapper);
    lv_label_set_text(region_prefix_label, "Region: ");
    lv_obj_align(region_prefix_label, LV_ALIGN_LEFT_MID, 0, 0);

    auto* region_label = lv_label_create(region_wrapper);
    std::string timeZoneName = time::getTimeZoneName();
    if (timeZoneName.empty()) {
        timeZoneName = "not set";
    }
    data->regionLabelWidget = region_label;
    lv_label_set_text(region_label, timeZoneName.c_str());
    // TODO: Find out why Y offset is needed
    lv_obj_align_to(region_label, region_prefix_label, LV_ALIGN_OUT_RIGHT_MID, 0, 8);

    auto* region_button = lv_button_create(region_wrapper);
    lv_obj_align(region_button, LV_ALIGN_TOP_RIGHT, 0, 0);
    auto* region_button_image = lv_image_create(region_button);
    lv_obj_add_event_cb(region_button, onConfigureTimeZonePressed, LV_EVENT_SHORT_CLICKED, nullptr);
    lv_image_set_src(region_button_image, LV_SYMBOL_SETTINGS);

    auto* time_format_wrapper= lv_obj_create(main_wrapper);
    lv_obj_set_width(time_format_wrapper, LV_PCT(100));
    lv_obj_set_height(time_format_wrapper, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(time_format_wrapper, 0, 0);
    lv_obj_set_style_border_width(time_format_wrapper, 0, 0);

    auto* time_24h_label = lv_label_create(time_format_wrapper);
    lv_label_set_text(time_24h_label, "24-hour clock");
    lv_obj_align(time_24h_label, LV_ALIGN_LEFT_MID, 0, 0);

    auto* time_24h_switch = lv_switch_create(time_format_wrapper);
    lv_obj_align(time_24h_switch, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(time_24h_switch, onTimeFormatChanged, LV_EVENT_VALUE_CHANGED, nullptr);
    if (time::isTimeFormat24Hour()) {
        lv_obj_add_state(time_24h_switch, LV_STATE_CHECKED);
    } else {
        lv_obj_remove_state(time_24h_switch, LV_STATE_CHECKED);
    }
}

static void onStart(AppContext& app) {
    auto data = std::make_shared<Data>();
    app.setData(data);
}

static void onResult(AppContext& app, Result result, const Bundle& bundle) {
    if (result == ResultOk) {
        auto data = std::static_pointer_cast<Data>(app.getData());
        auto name = timezone::getResultName(bundle);
        auto code = timezone::getResultCode(bundle);
        TT_LOG_I(TAG, "Result name=%s code=%s", name.c_str(), code.c_str());
        time::setTimeZone(name, code);

        if (!name.empty()) {
            if (lvgl::lock(100 / portTICK_PERIOD_MS)) {
                lv_label_set_text(data->regionLabelWidget, name.c_str());
                lvgl::unlock();
            }
        }
    }
}

extern const AppManifest manifest = {
    .id = "TimeDateSettings",
    .name = "Time & Date",
    .icon = TT_ASSETS_APP_ICON_TIME_DATE_SETTINGS,
    .type = TypeSettings,
    .onStart = onStart,
    .onShow = onShow,
    .onResult = onResult
};

void start() {
    service::loader::startApp(manifest.id);
}

} // namespace
