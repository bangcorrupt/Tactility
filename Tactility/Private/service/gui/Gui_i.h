#pragma once

#include "MessageQueue.h"
#include "Mutex.h"
#include "Pubsub.h"
#include "service/gui/Gui.h"
#include "service/gui/ViewPort.h"
#include "service/gui/ViewPort_i.h"
#include <cstdio>

namespace tt::service::gui {

#define GUI_THREAD_FLAG_DRAW (1 << 0)
#define GUI_THREAD_FLAG_INPUT (1 << 1)
#define GUI_THREAD_FLAG_EXIT (1 << 2)
#define GUI_THREAD_FLAG_ALL (GUI_THREAD_FLAG_DRAW | GUI_THREAD_FLAG_INPUT | GUI_THREAD_FLAG_EXIT)

/** Gui structure */
struct Gui {
    // Thread and lock
    Thread* thread = nullptr;
    Mutex mutex = Mutex(Mutex::TypeRecursive);
    PubSubSubscription* loader_pubsub_subscription = nullptr;

    // Layers and Canvas
    lv_obj_t* appRootWidget = nullptr;
    lv_obj_t* statusbarWidget = nullptr;

    // App-specific
    ViewPort* appViewPort = nullptr;

    lv_obj_t* _Nullable keyboard = nullptr;
    lv_group_t* keyboardGroup = nullptr;
};

/** Update GUI, request redraw */
void requestDraw();

/** Lock GUI */
void lock();

/** Unlock GUI */
void unlock();

} // namespace
