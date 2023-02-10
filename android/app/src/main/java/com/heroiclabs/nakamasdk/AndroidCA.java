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
import java.io.StringWriter;
import java.security.KeyStore;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;
import javax.net.ssl.TrustManagerFactory;
import org.bouncycastle.util.io.pem.PemObject;
import org.bouncycastle.util.io.pem.PemWriter;
import android.util.Log;

public class AndroidCA {

    @Keep
    public static byte[] getCaCertificates() {
        try {
            TrustManagerFactory tmf = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            tmf.init((KeyStore) null);
            X509Certificate[] trustedCertificates = new X509Certificate[0];
            for (javax.net.ssl.TrustManager tm : tmf.getTrustManagers()) {
                if (tm instanceof javax.net.ssl.X509TrustManager) {
                    // TODO so we just get the first? ssl trust manager shouldn't we get all of them?
                    trustedCertificates = ((javax.net.ssl.X509TrustManager) tm).getAcceptedIssuers();
                    break;
                }
            }
            StringBuilder sb = new StringBuilder();
            for (X509Certificate certificate : trustedCertificates) {
                try (PemWriter pemWriter = new PemWriter(new StringWriter())) {
                    pemWriter.writeObject(new PemObject("CERTIFICATE", certificate.getEncoded()));
                    pemWriter.flush();
                    sb.append(pemWriter.toString());
                }
            }

            return sb.toString().getBytes();
        } catch (Exception e) {
            Log.e("Nakama", "Unable to obtain trusted CA certificates: " + e.getMessage());
            return new byte[]{};
        }
    }
}
