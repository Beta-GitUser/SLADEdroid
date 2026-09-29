add_library(slade-core STATIC
	Common.cpp
	Memory.cpp
)

target_include_directories(slade-core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(slade-core PUBLIC cxx_std_17)
