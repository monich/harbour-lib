/*
 * Copyright (C) 2017-2020 Jolla Ltd.
 * Copyright (C) 2017-2020 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "HarbourMediaPlugin.h"
#include "HarbourDebug.h"

// Workaround for org.nemomobile.policy (or Sailfish.Media) not being
// allowed in harbour apps

HarbourMediaPlugin* HarbourMediaPlugin::gInstance = Q_NULLPTR;

const char HarbourMediaPlugin::MEDIAKEY_QML_TYPE[] = "MediaKey";

HarbourMediaPlugin::HarbourMediaPlugin(
    QQmlEngine* aEngine) :
    HarbourPluginLoader(aEngine, "Sailfish.Media", 1, 0)
{
}

HarbourMediaPlugin::~HarbourMediaPlugin()
{
    HASSERT(gInstance == this);
    gInstance = Q_NULLPTR;
}

void
HarbourMediaPlugin::registerTypes(
    const char* aModule,
    int aMajor,
    int aMinor)
{
    reRegisterType(MEDIAKEY_QML_TYPE, aModule, aMajor, aMinor);
}

void
HarbourMediaPlugin::registerTypes(
    QQmlEngine* aEngine,
    const char* aModule,
    int aMajor,
    int aMinor)
{
    if (!gInstance) {
        gInstance = new HarbourMediaPlugin(aEngine);
    }
    gInstance->registerTypes(aModule, aMajor, aMinor);
}
