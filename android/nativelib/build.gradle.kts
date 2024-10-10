plugins {
    id("com.android.library")
}

android {
    namespace = "com.heroiclabs.nakama"
    compileSdk = 34

    buildFeatures {
        prefabPublishing = true
    }

    prefab {
        create("nakama-sdk") {
            headers = ""
        }
    }

    defaultConfig {
        ndkVersion = "27.1.12297006"
        minSdk = 28
        externalNativeBuild {
            cmake {
                arguments("-DANDROID_STL=c++_shared")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path("../../CMakeLists.txt")
            version = "3.30.2"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
}

dependencies {
    implementation("androidx.appcompat:appcompat:1.7.0")
}