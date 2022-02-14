/*
 * If not stated otherwise in this file or this component's Licenses.txt file
 * the following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <rbus.h>

#define PRINT_EVENT(EVENT, SUBSCRIPTION) \
    printf("\n############################################################################\n" \
        " Event received in handler: %s\n" \
        " Subscription:\n" \
        "   eventName=%s\n" \
        "   userData=%s\n" \
        " Event:\n" \
        "   type=%d\n" \
        "   name=%s\n" \
        "   data=\n", \
            __FUNCTION__, \
            (SUBSCRIPTION)->eventName, \
            (char*)(SUBSCRIPTION)->userData, \
            (EVENT)->type, \
            (EVENT)->name); \
    rbusObject_fwrite((EVENT)->data, 8, stdout); \
    printf("\n############################################################################\n");


static void eventReceiveHandler(
    rbusHandle_t handle,
    rbusEvent_t const* event,
    rbusEventSubscription_t* subscription)
{
    (void)handle;

    rbusValue_t newValue = rbusObject_GetValue(event->data, "value");
    rbusValue_t oldValue = rbusObject_GetValue(event->data, "oldValue");
    printf("Consumer receiver ValueChange event for param %s\n", event->name);

    if(newValue)
        printf("   New Value: %d\n", rbusValue_GetInt32(newValue));

    if(oldValue)
        printf("   Old Value: %d\n", rbusValue_GetInt32(oldValue));

    if(subscription->userData)
        printf("   User data: %s\n", (char*)subscription->userData);

    PRINT_EVENT(event, subscription);

}

int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    int rc = RBUS_ERROR_SUCCESS;
    rbusHandle_t handle;
    rbusFilter_t filter;
    rbusValue_t filterValue;
    rbusEventSubscription_t subscription = {"Device.X_RDK_WanManager.CurrentActiveInterface", NULL, 0, 0, eventReceiveHandler, NULL, NULL, NULL};

    rc = rbus_open(&handle, "EventConsumer");
    if(rc != RBUS_ERROR_SUCCESS)
    {
        printf("consumer: rbus_open failed: %d\n", rc);
        return -1;
    }

    printf("Subscribing to Device.X_RDK_WanManager.CurrentActiveInterface\n");
    /* subscribe to all value change events on property "Device.Provider1.Param1" */
    rc = rbusEvent_Subscribe(
        handle,
        "Device.X_RDK_WanManager.CurrentActiveInterface",
        eventReceiveHandler,
        "My User Data",
        0);
    printf("After rbusEvent_Subscribe function...\n");

    sleep(30);

    /*printf("Unsubscribing Device.Provider1.Param1\n");

    rbusEvent_Unsubscribe(
        handle,
        "Device.X_RDK_WanManager.CurrentActiveInterface");
    printf("After rbusEvent_Unsubscribe function...\n");*/ 
   
    while(1);
    rbus_close(handle);
    return rc;
}

