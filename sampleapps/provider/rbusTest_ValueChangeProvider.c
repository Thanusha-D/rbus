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


rbusError_t getHandler(rbusHandle_t handle, rbusProperty_t property, rbusGetHandlerOptions_t* opts)
{
    printf("Entered into getHandler function...\n");
    char const* name = rbusProperty_GetName(property);
    (void)handle;
    (void)opts;

    /*fake a value change every 'myfreq' times this function is called*/
    static char* mydata[2] = {"erouter0", "LTE"};  /*the actual value to send back*/
    static int32_t mycount = -1;
    rbusValue_t value;
    mycount++;
    printf("Before if mycount = %d\n", mycount);
    if(mycount < 2)
    {
        printf("Called get handler for [%s] val=[%s]\n", name, mydata[mycount]);

        rbusValue_Init(&value);
        rbusValue_SetString(value, mydata[mycount]);
        rbusProperty_SetValue(property, value);
        rbusValue_Release(value);
    }
    else {
        return RBUS_ERROR_BUS_ERROR;
    }

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

    sleep(runtime);

    rbus_unregDataElements(handle, 1, dataElements);

exit1:
    rbus_close(handle);

exit2:
    printf("provider: exit\n");
    return rc;
}
