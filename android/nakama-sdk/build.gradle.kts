import java.nio.file.Files

plugins {
    id("com.android.library")
}

if (project.name != "nakama-sdk") {
    throw GradleException(
        "Subroject _MUST_ be named 'nakama-sdk', because that is how dependent projects CMake will find us"
    )
}

var headersDir = Files.createTempDirectory("headers")

// For speed, must be outside of regular build directory, otherwise CMake will recompile all every run
val cmakeBuildDir = project.layout.projectDirectory.dir(".cxx")

val copyH = tasks.register<Copy>( "copyConfigHeader") {
    into(headersDir.toFile())

    // Copy include files
    from("../../interface/include")

    // Copy generated config.h file from build directory.
    // There are multiple files matching pattern (from different build attempts and arches),
    // but it just happen to be the same content for all of them (lucky!) so we
    // copy essentially one at random.
    from(cmakeBuildDir) {
        include("**/interface/nakama-cpp/config.h")

        // Map all source files to the same destination path
        eachFile {
            path = "nakama-cpp/config.h"
        }
    }

    // Copy just first match as they are all the same
    duplicatesStrategy = DuplicatesStrategy.EXCLUDE
}

// rebuild native libs too after clean, why AGP doesn't do it by default?
tasks.clean {
    delete(cmakeBuildDir)
}

android {
    namespace = "com.heroiclabs.nakama"
    compileSdk = 34

    buildFeatures {
        prefabPublishing = true
    }

    defaultConfig {
        ndkVersion = "27.2.12479018"
        minSdk = 28
        externalNativeBuild {
            cmake {
                arguments("-DANDROID_STL=c++_shared", "-DINSIDE_GRADLE=ON")
                targets("nakama-sdk")
            }
        }
    }


    externalNativeBuild {
        cmake {
            path("../../CMakeLists.txt")
            version = "4.0.3"
            buildStagingDirectory = cmakeBuildDir.asFile
        }
    }

    prefab {
        register("nakama-sdk") {
            headers = headersDir.toString()
        }
    }

    // This is a prefab-only AAR, remove all jniLibs , which are just stripped version
    // of what we already include in prefab
    packaging {
         jniLibs.excludes += ("**/*.so")
    }

    buildToolsVersion = "36.0.0"
    ndkVersion = "27.2.12479018"
}


// Android plugin creates tasks after apply. These are not yet available in `tasks` during
// configure time, so we need to use `afterEvaluate` to find them.
afterEvaluate {
    // Copy headers after CMake build runs and generates config.h
    copyH.configure {
        val cmakeBuildTasks = tasks.matching { it.name.startsWith("buildCMake") }
        dependsOn(cmakeBuildTasks)
    }

    // prefab needs to wait until we copyH completes preparing directory with all headers
    tasks.matching {
        it.name.startsWith("prefab") && it.name.endsWith("ConfigurePackage")
    }.configureEach {
        dependsOn(copyH)
    }
}


dependencies {
    implementation("androidx.annotation:annotation:1.9.1")
}