plugins {
    id("com.android.library")
}

android {
    namespace = "com.heroiclabs.nakama"
    compileSdk = 34

    defaultConfig {
        ndkVersion = "27.2.12479018"
        minSdk = 28
        externalNativeBuild {
            cmake {
                arguments("-DANDROID_STL=c++_shared")
                targets("nakama-sdk")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path("../../CMakeLists.txt")
            version = "4.0.3"
        }
    }

    buildToolsVersion = "36.0.0"
    ndkVersion = "27.2.12479018"
}

dependencies {
        implementation("androidx.annotation:annotation:1.9.1")
}