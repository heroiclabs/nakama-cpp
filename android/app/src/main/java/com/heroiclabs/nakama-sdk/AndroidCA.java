/*
 * Copyright 2023 The Nakama Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.heroiclabs.nakamasdk;

import androidx.annotation.Keep;
import javax.net.ssl.TrustManagerFactory;
import java.security.KeyStore;
import android.util.Log;

public class AndroidCA {

    @Keep
    public static String getCAPath(String[] args) {
        try {
            TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            tmf.init((KeyStore) null);
            return tmf.getTrustManagers()[0];
        } catch (Exception e) {
            Log.e("nakama", "CA Path Error: " + e.msg);
            return env.get("ANDROID_ROOT") + "system/etc/security/cacerts"; // try a historically known path
        }
    }
}
