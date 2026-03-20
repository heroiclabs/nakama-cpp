import java.nio.file.Files

plugins {
    id("com.android.library")
}

// Subproject needs to be named 'nakama-sdk' 
// because that is how dependent CMake projects will find us
assertSubprojectName("nakama-sdk")

//
// Check cmake version, because we want 4+ here.
val systemCmakeVersion = getCmakeVersionString()
val cmakeMinimumVersion = "4.0.0"

assertMinimumVersion(systemCmakeVersion, cmakeMinimumVersion)


var headersDir = Files.createTempDirectory("headers")

// For speed, must be outside of regular build directory, otherwise CMake will recompile all every run
val cmakeBuildDir = project.layout.projectDirectory.dir(".cxx")

val copyH = tasks.register<Copy>("copyConfigHeader") {
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
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DINSIDE_GRADLE=ON",  // this is picked up by our CMake to interpose VCPKG toolchain
                    "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON",
                    "-DCMAKE_TOOLCHAIN_FILE=${project.rootDir}/../submodules/vcpkg/scripts/buildsystems/vcpkg.cmake",
                )
                targets("nakama-sdk")
            }
        }
    }

    compileOptions {
        targetCompatibility = JavaVersion.VERSION_11
        sourceCompatibility = JavaVersion.VERSION_11
    }

    externalNativeBuild {
        cmake {
            path("../../CMakeLists.txt")
            version = systemCmakeVersion
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
         // jniLibs.excludes += ("**/*.so")
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
        mustRunAfter(cmakeBuildTasks)
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

//
// Throws if subproject name differs from the given one.
fun assertSubprojectName(name: String) {
    if (project.name != name) {
        throw GradleException(
            "Subroject _MUST_ be named '${name}'."
        )
    }
}

//
// Gets a CMake version string from system-installed CMake. 
fun getCmakeVersionString(): String {
    //
    // Issue version command to CMake and get output.
    var cmakeVersionOutput: String
    try {
        cmakeVersionOutput = providers.exec {
            commandLine("cmake", "--version")
        }.standardOutput.asText.get()
    }
    catch (e: Exception) {
        throw GradleException("Unable to start CMake. Is it installed?")
    }

    //
    // CMake will generate multi-line output with
    // a version number in one of them, so we have to parse it.
    val versionRegex = Regex("""(\d+)\.(\d+)\.?(\d+)?""")
    val versionLines = cmakeVersionOutput.lines()

    //
    // Match every line against a regex and get the first match
    for (line in versionLines) {
        var match = versionRegex.find(line)
        match?.let {
            val (major, minor, patch) = it.destructured
            return "${major}.${minor}.${patch}"
        }
    }

    // If no match, something went wrong. Shouldn't happen.
    throw GradleException("Unable to get CMAKE version.")
}

//
// Throws an exception if `toAssert` represents
// an earlier version than `minimumVersion`
// Compares only major and minor versions.
// Assumes we will get proper version strings like '1.23.45'
fun assertMinimumVersion(toAssert: String, minimumVersion: String) {
    val (aMajor, aMinor) = toAssert.split('.').map { s -> s.toInt() }
    val (mMajor, mMinor) = minimumVersion.split('.').map { s -> s.toInt() }

    // If we have a bigger major version, we're good.
    if (aMajor > mMajor) {
        return
    }
    // If our major version is smaller, fail.
    // Otherwise (majors are the same), if our minor version is smaller, fail.
    else if (aMajor < mMajor || aMinor < mMinor) {
        throw GradleException(
            "Minimum version of CMAKE required is ${minimumVersion}. Version found: ${toAssert}"
        )
    }
}
