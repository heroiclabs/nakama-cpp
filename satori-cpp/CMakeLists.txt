set(SATORI_GIT_TAG "master" CACHE STRING "satori Git tag to fetch Proto files from")

file(GLOB_RECURSE SATORI_CPP_HEADERS
		include/*.h
)

add_subdirectory(src)
add_subdirectory(test EXCLUDE_FROM_ALL)
