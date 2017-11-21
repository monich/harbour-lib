/*
 * Copyright (C) 2017 Jolla Ltd.
 * Copyright (C) 2017 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HARBOUR_PLUGIN_LOADER_H
#define HARBOUR_PLUGIN_LOADER_H

#include <QString>

class QQmlEngine;
class QPluginLoader;

//
// This object has its most useful methods protected because it's supposed
// to be subclassed line this:
//
//   class DocGalleryPlugin : public HarbourPluginLoader {
//   public:
//     DocGalleryPlugin(QQmlEngine* engine);
//     void registerTypes(const char* module, int v1, int v2);
//   };
//
//   DocGalleryPlugin::DocGalleryPlugin(QQmlEngine* engine) :
//       HarbourPluginLoader(engine, "QtDocGallery", 5, 0)
//   {
//   }
//
//   void DocGalleryPlugin::registerTypes(const char* module, int v1, int v2)
//   {
//       reRegisterType("DocumentGallery", module, v1, v2);
//       reRegisterType("DocumentGalleryModel", module, v1, v2);
//       reRegisterType("GalleryStartsWithFilter", module, v1, v2);
//   }
//
// And it's generally a hack, it's best to avoid using this class at all.
//

class HarbourPluginLoader
{
public:
    bool isValid() const;

protected:
    HarbourPluginLoader(QQmlEngine* aEngine, QString aModule,
        int aMajor, int aMinor);
    ~HarbourPluginLoader();

    void reRegisterType(const char* aOrigQmlName, const char* aNewQmlName,
        const char* aModule, int aMajor, int aMinor);
    void reRegisterType(const char* aQmlName,
        const char* aModule, int aMajor, int aMinor);

private:
    class Private;
    Private* iPrivate;
};

#endif // HARBOUR_PLUGIN_LOADER_H
