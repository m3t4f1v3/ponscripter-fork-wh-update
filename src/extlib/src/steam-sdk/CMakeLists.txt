if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
		set(STEAM_BINARY "win64/steam_api64.dll")
	elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
		set(STEAM_BINARY "steam_api.dll")
	endif ()
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	# macOS binary is universal
	set(STEAM_BINARY "osx32/libsteam_api.dylib")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(STEAM_BINARY "linux64/libsteam_api.so")
	elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i.86")
		set(STEAM_BINARY "linux32/libsteam_api.so")
	endif ()
endif ()

if (STEAM_BINARY)
	add_library(Steam::Steam SHARED IMPORTED GLOBAL)
	set_target_properties(Steam::Steam PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/redistributable_bin/${STEAM_BINARY}")
	target_include_directories(Steam::Steam
		INTERFACE
			public/steam
			public)
else ()
	message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} with architecture ${CMAKE_SYSTEM_PROCESSOR} doesn't support steam!")
endif ()