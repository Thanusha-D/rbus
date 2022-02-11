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

//TODO handle filter matching

int runtime = 10;
int subscribed = 0;

rbusError_t eventSubHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish)
{
    (void)handle;
    (void)filter;
    (void)autoPublish;
    (void)interval;
    printf(
        "eventSubHandler called:\n" \
        "\taction=%s\n" \
        "\teventName=%s\n",
        action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribe" : "unsubscribe",
        eventName);
    printf("Before subscribed = %d\n", subscribed);
    if(!strcmp("Device.X_RDK_WanManager.CurrentActiveInterface", eventName))
    {
        subscribed = action == RBUS_EVENT_ACTION_SUBSCRIBE ? 1 : 0;
        printf("Inside if subcribe = %d\n", subscribed);
    }
    
    else
    {
        printf("provider: eventSubHandler unexpected eventName %s\n", eventName);
    }
    printf("After Subscribe = %d\n", subscribed);

    return RBUS_ERROR_SUCCESS;
}

rbusError_t getHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts)
{
    char const* name = rbusProperty_GetName(property);
    (void)handle;
    (void)opts;

    /*fake a value change every 'myfreq' times this function is called*/
    static int32_t mydata = 0;  /*the actual value to send back*/
    rbusValue_t value;

    mydata += 2;

    printf("Called get handler for [%s] val=[%d]\n", name, mydata);

    rbusValue_Init(&value);
    rbusValue_SetInt32(value, mydata);
    rbusProperty_SetValue(property, value);
    rbusValue_Release(value);

    return RBUS_ERROR_SUCCESS;
}

int main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);

    rbusHandle_t handle;
    int rc = RBUS_ERROR_SUCCESS;

    char componentName[] = "EventProvider";

    rbusDataElement_t dataElements[1] = {{"Device.X_RDK_WanManager.CurrentActiveInterface", RBUS_ELEMENT_TYPE_PROPERTY, {getHandler, NULL, NULL, NULL, eventSubHandler, NULL}}};

    printf("provider: start\n");

    rc = rbus_open(&handle, componentName);
    if(rc != RBUS_ERROR_SUCCESS)
    {
        printf("provider: rbus_open failed: %d\n", rc);
        goto exit2;
    }

    rc = rbus_regDataElements(handle, 1, dataElements);
    if(rc != RBUS_ERROR_SUCCESS)
    {
        printf("provider: rbus_regDataElements failed: %d\n", rc);
        goto exit1;
    }
    printf("Before publishing the event subscribe = %d\n", subscribed);
 
        if(subscribed)
        {
            rbusEvent_t event;
            rbusObject_t data;
            rbusValue_t value;

            printf("publishing Event Device.X_RDK_WanManager.CurrentActiveInterface subscribe = %d\n", subscribed);

            rbusValue_Init(&value);
            rbusValue_SetString(value, "rbus_vale_change");

            rbusObject_Init(&data, NULL);
            rbusObject_SetValue(data, "someText", value);

            event.name = dataElements[0].name;
            event.data = data;
            event.type = RBUS_EVENT_VALUE_CHANGED;

            rc = rbusEvent_Publish(handle, &event);

            rbusValue_Release(value);
            rbusObject_Release(data);

            if(rc != RBUS_ERROR_SUCCESS)
                printf("provider: rbusEvent_Publish Event1 failed: %d\n", rc);
        }

    printf("After publishing the event Device.X_RDK_WanManager.CurrentActiveInterface subscribe = %d\n", subscribed);
    sleep(runtime);

    rbus_unregDataElements(handle, 1, dataElements);

exit1:
    rbus_close(handle);

exit2:
    printf("provider: exit\n");
    return rc;
}
